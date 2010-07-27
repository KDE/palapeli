/***************************************************************************
 *   Copyright 2009, 2010 Stefan Majewsky <majewsky@gmx.net>
 *   Copyright 2010 Johannes Löhnert <loehnert.kde@gmx.de>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
***************************************************************************/

#include "piecevisuals.h"

#include <cmath>
#include <QDebug>
#include <QImage>
#include <QPainter>
#include <QtCore/qmath.h>

//BEGIN shadow blur algorithm

static void blur(QImage& image, const QRect& rect, int radius)
{
	const int tab[] = { 14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 };
	const int alpha = (radius < 1)  ? 16 : (radius > 17) ? 1 : tab[radius - 1];

	const int r1 = rect.top();
	const int r2 = rect.bottom();
	const int c1 = rect.left();
	const int c2 = rect.right();

	int bpl = image.bytesPerLine();
	int alphaChannel;
	unsigned char* p;

	for (int col = c1; col <= c2; col++) {
		p = image.scanLine(r1) + col * 4 + 3; //+3 to access alpha channel
		alphaChannel = *p << 4;

		p += bpl;
		for (int j = r1; j < r2; j++, p += bpl)
			*p = (alphaChannel += ((*p << 4) - alphaChannel) * alpha / 16) >> 4;
	}

	for (int row = r1; row <= r2; row++) {
		p = image.scanLine(row) + c1 * 4 + 3;
		alphaChannel = *p << 4;

		p += 4;
		for (int j = c1; j < c2; j++, p += 4)
			*p = (alphaChannel += ((*p << 4) - alphaChannel) * alpha / 16) >> 4;
	}

	for (int col = c1; col <= c2; col++) {
		p = image.scanLine(r2) + col * 4 + 3;
		alphaChannel = *p << 4;

		p -= bpl;
		for (int j = r1; j < r2; j++, p -= bpl)
				*p = (alphaChannel += ((*p << 4) - alphaChannel) * alpha / 16) >> 4;
	}

	for (int row = r1; row <= r2; row++) {
		p = image.scanLine(row) + c2 * 4 + 3;
			alphaChannel = *p << 4;

		p -= 4;
		for (int j = c1; j < c2; j++, p -= 4)
				*p = (alphaChannel += ((*p << 4) - alphaChannel) * alpha / 16) >> 4;
	}
}

static QImage makePixmapMonochrome(const QImage& image, const QColor& color)
{
	QImage baseImage(image.size(), QImage::Format_ARGB32_Premultiplied);
	baseImage.fill(color.rgba());

	QImage monoImage(image);
	QPainter px(&monoImage);
	px.setCompositionMode(QPainter::CompositionMode_SourceAtop);
	px.drawImage(0, 0, baseImage);
	px.end();
	return monoImage;
}

static QImage createShadow(const QImage& source, int radius)
{
	QImage shadowImage(QSize(source.width() + 2 * radius, source.height() + 2 * radius), QImage::Format_ARGB32_Premultiplied);
	shadowImage.fill(0x00000000); //transparent
	QPainter px(&shadowImage);
	px.drawImage(QPoint(radius, radius), makePixmapMonochrome(source, Qt::black));
	px.end();

	blur(shadowImage, QRect(QPoint(), shadowImage.size()), radius / 3);
	return shadowImage;
}

//END shadow blur algorithm

//BEGIN edge beveling algorithms

// See piecevisuals.h for some explanations about BevelMap.
Palapeli::BevelMap Palapeli::calculateBevelMap(const Palapeli::PieceVisuals& source, int radius)
{
	const qreal strength_scale = 0.2;
	// in multiples of radius
	const qreal outline_width = 0.07;
	const qreal outline_darken_scale = 2;

	const QImage img = source.image().convertToFormat(QImage::Format_ARGB32);
	const int width = img.width();
	const int height = img.height();

	// prepare the folding kernels (access order: x * radius + y)
	QVector<qreal> kernel_bevel(radius * radius, 0.0);
	QVector<qreal> kernel_outline(radius * radius, 0.0);
	for (int n_xf = 0, n_fIndex = 0; n_xf < radius; ++n_xf)
	{
		for (int n_yf = 0; n_yf < radius; ++n_yf, ++n_fIndex)
		{
			qreal xfold = n_xf + 0.5;
			qreal yfold = n_yf + 0.5;
			qreal len = xfold*xfold + yfold*yfold;
			if (len > radius*radius) continue;
			len = qSqrt(len);
			// 3/pi/radius: normalization so that sum over all elements is (roughly) 1/r
			// (1 - len/radius)^2 : actual blur falloff function
			qreal t = (1 - len/radius);
			kernel_bevel[n_fIndex] = 3.0 / (M_PI*radius) * (t*t);
			// outline kernel
			if (len >= radius*outline_width + 0.5) continue;
			if (len <= radius*outline_width - 0.5)
			{
				kernel_outline[n_fIndex] = 1.0 / (M_PI*radius);
			}
			else
			{
				// roughly estimate coverage of that pixel
				kernel_outline[n_fIndex] = (radius*outline_width + 0.5 - len) / (M_PI*radius);
			}
		}
	}

	// stores the blurred "fallof" of the transparency at each point.
	QVector<QPointF> vmap(width*height);
	// stores how much pixel shall be darkened (outline effect)
	QVector<qreal> darkening(width*height);
	// cache scanLine pointers (these are used very often from now on)
	QVector<QRgb*> scanLinePointers(height);
	for (int ny = 0; ny < height; ++ny)
		scanLinePointers[ny] = (QRgb*) img.scanLine(ny);

	// iterate over img's pixel, finding the alpha edges
	// we imagine the image padded into a 1px wide transparent border,
	// thus iteration starts at -1.
	for (int ny = -1; ny < height; ++ny)
	{
		for (int nx = -1; nx < width; ++nx)
		{
			// for each edge, we add a blurred vector "blob" (the folding kernel_bevel) into the target vector at the right spot.
			// find alpha values
			// The conditions in here are a condensed version of img.rect().contains(nx(+1),ny(+1)).
			const int a1 = (nx == -1 || ny == -1) ? 0 : qAlpha(scanLinePointers[ny][nx]);
			const int a2 = (nx == width-1 || ny == -1) ? 0 : qAlpha(scanLinePointers[ny][nx+1]);
			const int a3 = (nx == -1 || ny == height-1) ? 0 : qAlpha(scanLinePointers[ny+1][nx]);
			const int a4 = (nx == width-1 || ny == height-1) ? 0 : qAlpha(scanLinePointers[ny+1][nx+1]);
			// find difference
			const int dx = (a2 + a4) - (a1 + a3);
			const int dy = (a3 + a4) - (a1 + a2);
			int diff = dx * dx + dy * dy;
			if (diff == 0)
				continue;
			diff = qSqrt(diff);

			// iterate over the folding kernel_bevel
			for (int n_xf = 0, n_fIndex = 0; n_xf < radius; ++n_xf)
			{
				//break statement in second loop might break n_fIndex
				n_fIndex = n_xf * radius;
				for (int n_yf = 0; n_yf < radius; ++n_yf, ++n_fIndex)
				{
					const qreal bevelfactor = kernel_bevel[n_fIndex];
					if (bevelfactor == 0.0)
						// only 0 will follow. break n_yf iteration and proceed with next n_xf.
						break;
					const qreal darkenfactor = diff * kernel_outline[n_fIndex];
					for (int quadrant=0; quadrant<4; ++quadrant) {
						// the coordinates of the point we write to
						const int n_xtick = (quadrant%2 == 0) ? nx - n_xf : nx + 1 + n_xf;
						const int n_ytick = (quadrant/2 == 0) ? ny - n_yf : ny + 1 + n_yf;
						//if (image.rect().contains(n_xtick, n_ytick))
						if (n_xtick>=0 && n_ytick>=0 && n_xtick < width && n_ytick < height)
						{
							const int n_tickIndex = n_xtick + n_ytick * width;
							vmap[n_tickIndex] += QPointF(bevelfactor*dx, bevelfactor*dy);
							darkening[n_tickIndex] += darkenfactor;
						}
					}
				}
			} // end of iteration over folding kernel
		}
	} // end of iteration over source

	// re-encode the result into polar coordinates / RLE zero runs as described in piecevisuals.h.
	// While at it, we wipe out all elements where source alpha is zero.
	Palapeli::BevelMap result(width*height);
	int last_nonzero = -1;
	int strength;

	for (int y = 0, idx = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x, ++idx)
		{
			const QRgb srccolor = scanLinePointers[y][x];
			if (qAlpha(srccolor) == 0)
				vmap[idx] = QPointF(0.0, 0.0);

			QLineF l(QPointF(0.0, 0.0), vmap[idx]);
			strength = l.length() * strength_scale;
			if (strength>255) strength=255;

			if (strength>0) {
				if (last_nonzero < idx-1)
				{
					result[last_nonzero+1].strength = 0;
					// orig_argb stores run length
					result[last_nonzero+1].orig_argb = idx - last_nonzero - 1;
				}
				last_nonzero = idx;
				result[idx].strength = strength;
				result[idx].angle = int(l.angle() * 256 / 360 + 256) % 256;
				QColor color = QColor::fromRgba(srccolor);
				if (darkening[idx] > 0) color = color.darker(100 + darkening[idx]*outline_darken_scale);
				result[idx].orig_argb = color.rgba();
			}
		}
	}
	// encode last zero run if present
	if (last_nonzero < width*height-1)
	{
		result[last_nonzero+1].strength = 0;
		// orig_argb stores run length
		result[last_nonzero+1].orig_argb = width*height - last_nonzero - 1;
	}
	return result;
}

// source: unbeveled image
// bevelmap: must belong to the pixmap
// angle: rotation angle of source, 0=unrotated, +x = ccw
QImage Palapeli::applyBevelMap(const QImage &source, const Palapeli::BevelMap& bevelmap, qreal angle)
{
	// first, prevent mem corruption
	if (source.width() * source.height() != bevelmap.size())
	{
		qWarning() << "ApplyBevelMap: returning unbeveled source since bevelmap has wrong size";
		return source;
	}

	QImage result = source.convertToFormat(QImage::Format_ARGB32);
	const int width = result.width();
	const int height = result.height();
	const int size = width * height;
	// in degrees
	const qreal lighting_angle = 120;

	// convert into 256-unit circle
	angle = (lighting_angle - angle) * 256 / 360;

	int scanLineY = -1;
	QRgb* scanLine = 0;
	for (int idx = 0; idx < size; ++idx)
	{
		const int x = idx % width, y = idx / width;
		if (scanLineY != y)
			scanLine = (QRgb*) result.scanLine(y);
		const Palapeli::BevelPoint bevelpoint = bevelmap[idx];
		if (bevelpoint.strength == 0)
		{
			// zero run, skip to next nonzero element
			idx += int(bevelpoint.orig_argb);
			continue;
		}
		const int strength = bevelpoint.strength;
		int adiff = bevelpoint.angle - angle;
		// project adiff into [0, 255]
		adiff = adiff & 0xff;

		// cosine-like function: concatenation of
		// parabolas, which looks almost the same
		// and is faster to calculate
		// 128 = 180°, 255 = 359°, 90 = 64°
		// project adiff into [-64, 63]
		if (adiff >= 128)
			adiff = 255 - adiff;
		if (adiff >= 64)
		{
			// adiff<0 will be the signal to signal lighten instead of darken
			adiff = adiff-128;
		}
		const qreal effect = strength * (1.0 - (adiff*adiff) / (64.0*64.0));

		QColor newcolor = QColor::fromRgba(bevelpoint.orig_argb);
		if (adiff<0)
			newcolor = newcolor.lighter(100 + effect);
		else
			newcolor = newcolor.darker(100 + effect);
		scanLine[x] = newcolor.rgba();
	}
	return result;
}
//END edge beveling algorithms

Palapeli::PieceVisuals Palapeli::createShadow(const Palapeli::PieceVisuals& pieceVisuals, const QSize& shadowSizeHint)
{
	const QSize shadowSizeHintUse = shadowSizeHint.isEmpty() ? pieceVisuals.size() : shadowSizeHint;
	const int shadowRadius = 0.15 * (shadowSizeHintUse.width() + shadowSizeHintUse.height());
	return Palapeli::PieceVisuals(
		createShadow(pieceVisuals.image(), shadowRadius),
		pieceVisuals.offset() - QPoint(shadowRadius, shadowRadius)
	);
}

Palapeli::PieceVisuals Palapeli::changeShadowColor(const Palapeli::PieceVisuals& shadowVisuals, const QColor& color)
{
	return Palapeli::PieceVisuals(
		makePixmapMonochrome(shadowVisuals.image(), color),
		shadowVisuals.offset()
	);
}

Palapeli::PieceVisuals Palapeli::mergeVisuals(const QList<Palapeli::PieceVisuals>& visuals)
{
	//determine geometry of combined pixmap, and hold a vote on which representation to use
	int imageCount = 0, pixmapCount = 0;
	QRect combinedGeometry;
	foreach (const Palapeli::PieceVisuals& sample, visuals)
	{
		QRect sampleGeometry(sample.offset(), sample.size());
		if (combinedGeometry.isNull())
			combinedGeometry = sampleGeometry;
		else
			combinedGeometry |= sampleGeometry;
		//NOTE: bool-int cast always returns 0 or 1.
		imageCount += (int) sample.hasImage();
		pixmapCount += (int) sample.hasPixmap();
	}
	const QPoint combinedOffset = combinedGeometry.topLeft();
	//combine pixmaps
	if (pixmapCount > imageCount)
	{
		QPixmap combinedPixmap(combinedGeometry.size());
		combinedPixmap.fill(Qt::transparent);
		QPainter painter(&combinedPixmap);
		foreach (const Palapeli::PieceVisuals& sample, visuals)
			painter.drawPixmap(sample.offset() - combinedOffset, sample.pixmap());
		painter.end();
		return Palapeli::PieceVisuals(combinedPixmap, combinedOffset);
	}
	else
	{
		QImage combinedImage(combinedGeometry.size(), QImage::Format_ARGB32_Premultiplied);
		combinedImage.fill(0x00000000); // == Qt::transparent
		QPainter painter(&combinedImage);
		foreach (const Palapeli::PieceVisuals& sample, visuals)
			painter.drawImage(sample.offset() - combinedOffset, sample.image());
		painter.end();
		return Palapeli::PieceVisuals(combinedImage, combinedOffset);
	}
}

Palapeli::BevelMap Palapeli::mergeBevelMaps(const QList<Palapeli::PieceVisuals>& visuals, const QList<Palapeli::BevelMap>& bevelMaps)
{
	//determine geometry of combined pixmap
	QRect combinedGeometry;
	foreach (const Palapeli::PieceVisuals& sample, visuals)
	{
		QRect sampleGeometry(sample.offset(), sample.size());
		if (combinedGeometry.isNull())
			combinedGeometry = sampleGeometry;
		else
			combinedGeometry |= sampleGeometry;
	}
	const QPoint combinedOffset = combinedGeometry.topLeft();
	const int combinedWidth = combinedGeometry.width();
	const int combinedHeight = combinedGeometry.height();

	Palapeli::BevelMap result(combinedWidth*combinedHeight);
	for (int i=0; i<visuals.size(); i++)
	{
		int srcwidth = visuals[i].size().width();
		QPoint offset = visuals[i].offset() - combinedOffset;
		for (int srcidx=0; srcidx<bevelMaps[i].size(); srcidx++)
		{
			if (bevelMaps[i][srcidx].strength==0)
			{
				// zero run, skip
				srcidx += int(bevelMaps[i][srcidx].orig_argb);
				if (srcidx > bevelMaps[i].size())
					break;
			}
			QPoint dst = QPoint(srcidx%srcwidth, srcidx/srcwidth);
			dst += offset;
			if (dst.x() >= 0 && dst.y() >= 0 && dst.x() < combinedWidth && dst.y() < combinedHeight)
			{
				int dstidx = dst.x() + dst.y() * combinedWidth;
				if (result[dstidx].strength == 0)
				{
					result[dstidx] = bevelMaps[i][srcidx];
				}
				else
				{
					int a1 = qAlpha(result[dstidx].orig_argb);
					int a2 = qAlpha(bevelMaps[i][srcidx].orig_argb);
					// pixel with most opacity wins
					if (a2 > a1) result[dstidx] = bevelMaps[i][srcidx];
					QColor color = QColor::fromRgba(result[dstidx].orig_argb);
					// compensate premultiplication of components
					color = color.lighter(255 * 100 / color.alpha());
					result[dstidx].orig_argb = color.rgb();
					
					// reduce strength if alpha difference is low, to avoid aliasing at the edges.
					int strength = result[dstidx].strength;
					strength = strength * (a1-a2) / 255;
					if (strength==0)
						strength=1;
					else
						strength = (strength<0) ? -strength: strength;
					result[dstidx].strength = strength;
				}
			}
		}
	}

	// Run-length encode the zero runs
	int last_nonzero = -1;
	for (int dstidx=0; dstidx<result.size(); dstidx++)
	{
		if (result[dstidx].strength!=0)
		{
			if (last_nonzero < dstidx-1)
				result[last_nonzero+1].orig_argb = dstidx - last_nonzero - 1;
			last_nonzero = dstidx;
		}
	}
	if (last_nonzero < result.size()-1)
		result[last_nonzero+1].orig_argb = result.size() - last_nonzero - 1;
	
	return result;
}

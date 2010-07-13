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
#include <QApplication>
#include <QDebug>
#include <QImage>
#include <QPainter>
#include <QPalette>
#include <QPropertyAnimation>

//BEGIN shadow blur algorithm

static void blur(QImage& image, const QRect& rect, int radius)
{
	int tab[] = { 14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 };
	int alpha = (radius < 1)  ? 16 : (radius > 17) ? 1 : tab[radius - 1];

	int r1 = rect.top();
	int r2 = rect.bottom();
	int c1 = rect.left();
	int c2 = rect.right();

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

static QPixmap makePixmapMonochrome(const QPixmap& pixmap, const QColor& color)
{
	QPixmap basePixmap(pixmap.size());
	basePixmap.fill(color);

	QPixmap monoPixmap(pixmap);
	QPainter px(&monoPixmap);
	px.setCompositionMode(QPainter::CompositionMode_SourceAtop);
	px.drawPixmap(0, 0, basePixmap);
	px.end();
	return monoPixmap;
}

static QPixmap createShadow(const QPixmap& source, int radius)
{
	QPixmap shadowPixmap = makePixmapMonochrome(source, Qt::black);

	QImage shadowImage(QSize(source.width() + 2 * radius, source.height() + 2 * radius), QImage::Format_ARGB32_Premultiplied);
	shadowImage.fill(0x00000000); //transparent
	QPainter px(&shadowImage);
	px.drawPixmap(QPoint(radius, radius), shadowPixmap);
	px.end();

	blur(shadowImage, QRect(QPoint(), shadowImage.size()), radius / 3);
	return QPixmap::fromImage(shadowImage);
}

//END shadow blur algorithm

//BEGIN edge beveling algorithms

// See piecevisuals.h for some explanations about BevelMap.
Palapeli::BevelMap Palapeli::calculateBevelMap(const QPixmap& source, int radius)
{
	const qreal strength_scale = 0.2;

	QImage sourceimg = source.toImage();
	// we only really care about the alpha channel here. And we expect ARGB32_Premultiplied anyway.
	if (sourceimg.format() != QImage::Format_ARGB32 && sourceimg.format() != QImage::Format_ARGB32_Premultiplied)
		sourceimg = sourceimg.convertToFormat(QImage::Format_ARGB32);

	// make img const to avoid automatic copying when accessing pixels.
	const QImage img = sourceimg;

	int width = img.width();
	int height = img.height();

	// prepare the folding kernel
	qreal** kernel = new qreal*[radius];
	for (int n_xf=0; n_xf<radius; n_xf++)
	{
		kernel[n_xf] = new qreal[radius];
		for (int n_yf=0; n_yf<radius; n_yf++)
		{
			qreal xfold = n_xf + 0.5;
			qreal yfold = n_yf + 0.5;
			qreal len = xfold*xfold + yfold*yfold;
			kernel[n_xf][n_yf] = 0.0;
			if (len > radius*radius) continue;
			len = sqrt(len);
			// 3/pi/radius^2: normalization so that sum over all elements is (roughly) 1
			// or precisely 1/4th, since we calculate only 1 quadrant
			// (1 - len/radius)^2 : actual blur falloff function
			qreal t = (1 - len/radius);
			kernel[n_xf][n_yf] = 3.0 / (M_PI*radius) * (t*t);
		}
	}

	// stores the blurred "fallof" of the transparency at each point.
	QVector<QPointF> vmap(width*height);

	// iterate over img's pixel, finding the alpha edges
	// we imagine the image padded into a 1px wide transparent border,
	// thus iteration starts at -1.
	QRect rect = img.rect();
	for (int nx=-1; nx<width; nx++)
	{
		for (int ny=-1; ny<height; ny++)
		{
			// for each edge, we add a blurred vector "blob" (the folding kernel) into the target vector at the right spot.
			// the alpha values
			int a1=0, a2=0, a3=0, a4=0;
			if (rect.contains(nx, ny))
				a1 = qAlpha(* (QRgb*)(img.scanLine(ny) + 4*nx));
			if (rect.contains(nx+1, ny))
				a2 = qAlpha(* (QRgb*)(img.scanLine(ny) + 4*nx + 4));
			if (rect.contains(nx, ny+1))
				a3 = qAlpha(* (QRgb*)(img.scanLine(ny+1) + 4*nx));
			if (rect.contains(nx+1, ny+1))
				a4 = qAlpha(* (QRgb*)(img.scanLine(ny+1) + 4*nx + 4));

			// find difference
			int dx = (a2 + a4) - (a1 + a3);
			int dy = (a3 + a4) - (a1 + a2);

			if (dx==0 && dy==0) continue;

			// iterate over the folding kernel
			for (int n_xf = 0; n_xf < radius; n_xf++)
			{
				for (int n_yf = 0; n_yf < radius; n_yf++)
				{
					qreal factor = kernel[n_xf][n_yf];
					if (factor == 0.0)
						// only 0 will follow. break n_yf iteration and proceed with next n_xf.
						break;
					for (int quadrant=0; quadrant<4; quadrant++) {
						// the coordinates of the point we write to
						int n_xtick = (quadrant%2 == 0) ? nx - n_xf : nx + 1 + n_xf;
						int n_ytick = (quadrant/2 == 0) ? ny - n_yf : ny + 1 + n_yf;
						//if (rect.contains(n_xtick, n_ytick))
						if (n_xtick>=0 && n_ytick>=0 && n_xtick < width && n_ytick < height)
						{
							vmap[n_xtick + n_ytick * width] += QPointF(factor*dx, factor*dy);
						}
					}
				}
			} // end of iteration over folding kernel
		}
	} // end of iteration over source

	// re-encode the result into polar coordinates / RLE zero runs as described in piecevisuals.h.
	// While at it, we wipe out all elements where source alpha is zero.
	Palapeli::BevelMap result = Palapeli::BevelMap(width*height);
	int last_nonzero = -1;
	int strength;

	for (int idx = 0; idx < width*height; idx++)
	{
		int srcalpha = qAlpha(* (QRgb*)(img.scanLine(idx/width) + 4*(idx%width)));
		if (srcalpha == 0)
			vmap[idx] = QPointF(0.0, 0.0);

		QLineF l = QLineF(QPointF(0.0, 0.0), vmap[idx]);
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
			result[idx].orig_argb = *((QRgb*)(img.scanLine(idx/width) + 4*(idx%width)));
		}
	}
	// encode last zero run if present
	if (last_nonzero < width*height-1)
	{
				result[last_nonzero+1].strength = 0;
				// orig_argb stores run length
				result[last_nonzero+1].orig_argb = width*height - last_nonzero - 1;
	}

	// cleanup
	for (int n_xf=0; n_xf < radius; n_xf ++) {
		delete[] kernel[n_xf];
	}
	delete[] kernel;

	return result;
}

// source: unbeveled pixmap
// bevelmap: must belong to the pixmap
// angle: rotation angle of source, 0=unrotated, +x = ccw
QPixmap Palapeli::applyBevelMap(const QPixmap &source, const Palapeli::BevelMap& bevelmap, qreal angle)
{
	// first, prevent mem corruption
	if (source.width() * source.height() != bevelmap.size())
	{
		qWarning() << "ApplyBevelMap: returning unbeveled source since bevelmap has wrong size";
		return source;
	}

	QImage result = source.toImage();
	result.detach();
	int width = result.width();
	// in degrees
	const qreal lighting_angle = 120;

	// convert into 256-unit circle
	angle = (lighting_angle - angle) * 256 / 360;

	for (int idx=0; idx < bevelmap.size(); idx++) {
		if (bevelmap[idx].strength == 0)
		{
			// zero run, skip to next nonzero element
			idx += int(bevelmap[idx].orig_argb);
			if (idx >= bevelmap.size()) 
				break;
		}
		int strength = bevelmap[idx].strength;
		int adiff = bevelmap[idx].angle - angle;
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
		qreal effect = strength * (1.0 - (adiff*adiff) / (64.0*64.0));

		QColor newcolor = QColor::fromRgba(bevelmap[idx].orig_argb);
		if (adiff<0)
		{
			newcolor = newcolor.lighter(100 + effect);
		} else {
			newcolor = newcolor.darker(100 + effect);
		}
		*(QRgb*)(result.scanLine(idx/width) + 4*(idx%width)) = newcolor.rgba();
	}

	return QPixmap::fromImage(result);
}
//END edge beveling algorithms

Palapeli::PieceVisuals Palapeli::createShadow(const Palapeli::PieceVisuals& pieceVisuals, const QSize& shadowSizeHint)
{
	const QSize shadowSizeHintUse = shadowSizeHint.isEmpty() ? pieceVisuals.pixmap.size() : shadowSizeHint;
	const int shadowRadius = 0.15 * (shadowSizeHintUse.width() + shadowSizeHintUse.height());
	Palapeli::PieceVisuals result;
	result.pixmap = createShadow(pieceVisuals.pixmap, shadowRadius);
	result.offset = pieceVisuals.offset - QPoint(shadowRadius, shadowRadius);
	return result;
}

Palapeli::PieceVisuals Palapeli::changeShadowColor(const Palapeli::PieceVisuals& shadowVisuals, const QColor& color)
{
	Palapeli::PieceVisuals result = shadowVisuals;
	result.pixmap = makePixmapMonochrome(result.pixmap, color);
	return result;
}

Palapeli::PieceVisuals Palapeli::mergeVisuals(const QList<Palapeli::PieceVisuals>& visuals)
{
	//determine geometry of combined pixmap
	QRect combinedGeometry;
	foreach (const Palapeli::PieceVisuals& sample, visuals)
	{
		QRect sampleGeometry(sample.offset, sample.pixmap.size());
		if (combinedGeometry.isNull())
			combinedGeometry = sampleGeometry;
		else
			combinedGeometry |= sampleGeometry;
	}
	//combine pixmaps
	const QPoint combinedOffset = combinedGeometry.topLeft();
	QPixmap combinedPixmap(combinedGeometry.size());
	combinedPixmap.fill(Qt::transparent);
	QPainter painter(&combinedPixmap);
	foreach (const Palapeli::PieceVisuals& sample, visuals)
		painter.drawPixmap(sample.offset - combinedOffset, sample.pixmap);
	painter.end();
	//create result
	Palapeli::PieceVisuals result = { combinedPixmap, combinedOffset };
	return result;
}

Palapeli::BevelMap Palapeli::mergeBevelMaps(const QList<Palapeli::PieceVisuals>& visuals, const QList<Palapeli::BevelMap>& bevelMaps)
{
	//determine geometry of combined pixmap
	QRect combinedGeometry;
	foreach (const Palapeli::PieceVisuals& sample, visuals)
	{
		QRect sampleGeometry(sample.offset, sample.pixmap.size());
		if (combinedGeometry.isNull())
			combinedGeometry = sampleGeometry;
		else
			combinedGeometry |= sampleGeometry;
	}
	const QPoint combinedOffset = combinedGeometry.topLeft();
	const int combinedWidth = combinedGeometry.width();
	const int combinedHeight = combinedGeometry.height();

	BevelPoint p;
	p.strength=0;
	p.angle=0;
	p.orig_argb=0;
	Palapeli::BevelMap result(combinedWidth*combinedHeight, p);
	for (int i=0; i<visuals.size(); i++)
	{
		int srcwidth = visuals[i].pixmap.width();
		QPoint offset = visuals[i].offset - combinedOffset;
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
					result[dstidx].strength = 0;
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

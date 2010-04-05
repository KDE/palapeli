/***************************************************************************
 *   Copyright 2009, 2010 Stefan Majewsky <majewsky@gmx.net>
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

#include "newpiecevisuals.h"

#include <cmath>
#include <QImage>
#include <QApplication>
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

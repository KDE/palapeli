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

QPixmap Palapeli::makePixmapMonochrome(const QPixmap& pixmap, const QColor& color)
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

QPixmap Palapeli::createShadow(const QPixmap& source, int radius)
{
	QPixmap shadowPixmap = Palapeli::makePixmapMonochrome(source, Qt::black);

	QImage shadowImage(QSize(source.width() + 2 * radius, source.height() + 2 * radius), QImage::Format_ARGB32_Premultiplied);
	shadowImage.fill(0x00000000); //transparent
	QPainter px(&shadowImage);
	px.drawPixmap(QPoint(radius, radius), shadowPixmap);
	px.end();

	blur(shadowImage, QRect(QPoint(), shadowImage.size()), radius / 3);
	return QPixmap::fromImage(shadowImage);
}

//END shadow blur algorithm

Palapeli::ShadowItem::ShadowItem(const QPixmap& pixmap, int radius, const QPointF& offset)
	: m_baseShadow(new QGraphicsPixmapItem(this))
	, m_activeShadow(new QGraphicsPixmapItem(this))
	, m_animator(new QPropertyAnimation(this, "activeOpacity", this))
{
	//create shadows
	const QPixmap baseShadowPixmap = Palapeli::createShadow(pixmap, radius);
	const QColor activeShadowColor = QApplication::palette().color(QPalette::Highlight);
	const QPixmap activeShadowPixmap = Palapeli::makePixmapMonochrome(baseShadowPixmap, activeShadowColor);
	//setup items
	m_baseShadow->setPixmap(baseShadowPixmap);
	m_baseShadow->setAcceptedMouseButtons(Qt::LeftButton);
	m_baseShadow->setOffset(offset + QPointF(-radius, -radius));
	m_activeShadow->setPixmap(activeShadowPixmap);
	m_activeShadow->setAcceptedMouseButtons(Qt::LeftButton);
	m_activeShadow->setOffset(offset + QPointF(-radius, -radius));
	m_activeShadow->setOpacity(0.0);
}

qreal Palapeli::ShadowItem::activeOpacity() const
{
	return m_activeShadow->opacity();
}

void Palapeli::ShadowItem::setActiveOpacity(qreal activeOpacity)
{
	m_activeShadow->setOpacity(activeOpacity);
// 	m_baseShadow->setOpacity(1.0 - activeOpacity);
}

void Palapeli::ShadowItem::setActive(bool active)
{
	const qreal targetOpacity = active ? 1.0 : 0.0;
	if (targetOpacity == activeOpacity())
		return;
	m_animator->setDuration(200 * qAbs(targetOpacity - activeOpacity()));
	m_animator->setStartValue(activeOpacity());
	m_animator->setEndValue(targetOpacity);
	m_animator->start();
}

#include "newpiecevisuals.moc"

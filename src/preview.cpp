/***************************************************************************
 *   Copyright (C) 2008 Felix Lemke <lemke.felix@ages-skripte.org>
 *   Copyright (C) 2008 Stefan Majewsky <majewsky@gmx.net>
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

#include "preview.h"

#include <QImage>
#include <QPainter>

Palapeli::Preview::Preview()
	: QWidget()
	, m_image(new QImage(200, 200, QImage::Format_ARGB32))
{
	setMinimumSize(200, 200);
}

Palapeli::Preview::~Preview()
{}

void Palapeli::Preview::loadImage(const QString& file)
{
	m_image->load(file);
	repaint();
}

void Palapeli::Preview::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	const qreal scalingFactorHorizontal = (qreal) this->width() / (qreal) m_image->width();
	const qreal scalingFactorVertical = (qreal) this->height() / (qreal) m_image->height();
	if (scalingFactorHorizontal > scalingFactorVertical)
		painter.scale(scalingFactorVertical, scalingFactorVertical);
	else
		painter.scale(scalingFactorHorizontal, scalingFactorHorizontal);
	painter.drawImage(0, 0, *m_image);
}

#include "preview.moc"

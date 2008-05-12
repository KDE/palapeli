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

#include <QPainter>

Palapeli::Preview::Preview(QWidget* parent)
	: QWidget(parent)
	, m_image()
{
	setMinimumSize(150, 150);
}

void Palapeli::Preview::setImage(const QImage &image)
{
	m_image = image;
	update();
}

void Palapeli::Preview::paintEvent(QPaintEvent*)
{
	if (m_image.isNull())
		return;
	qreal scalingFactor = qMin((qreal) width() / m_image.width(), (qreal) height() / m_image.height());
	QPainter painter(this);
	painter.scale(scalingFactor, scalingFactor);
	painter.drawImage(QPoint(0, 0), m_image);
}

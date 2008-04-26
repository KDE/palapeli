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

#include "piece.h"
#include "manager.h"
#include "part.h"
#include "view.h"

#include <QGraphicsSceneMouseEvent>

Palapeli::Piece::Piece(const QPixmap& pixmap, const QSize& size, Palapeli::Manager* manager)
	: QGraphicsPixmapItem()
	, m_manager(manager)
	, m_part(0)
	, m_size(size)
	, m_moving(false)
{
	setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
	setPixmap(pixmap);
	setOffset(0.0, 0.0);

	m_manager->view()->scene()->addItem(this);
	QRectF pieceRect = sceneBoundingRect();
	scale(size.width() / pieceRect.width(), size.height() / pieceRect.height());
}

Palapeli::Part* Palapeli::Piece::part() const
{
	return m_part;
}

void Palapeli::Piece::setPart(Palapeli::Part* part)
{
	m_part = part;
}

QSize Palapeli::Piece::size() const
{
	return m_size;
}

void Palapeli::Piece::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if (m_part)
		m_moving = event->button() & Qt::LeftButton;
}

void Palapeli::Piece::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	if (m_part && m_moving)
		m_part->move(event->scenePos() - event->lastScenePos());
}

void Palapeli::Piece::mouseReleaseEvent(QGraphicsSceneMouseEvent*)
{
	if (m_part && m_moving)
	{
		m_manager->searchConnections();
		m_moving = false;
	}
}

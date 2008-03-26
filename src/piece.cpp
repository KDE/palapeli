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
#include "part.h"
#include "scene.h"

#include <QGraphicsSceneMouseEvent>

Palapeli::Piece::NeighborInfo::NeighborInfo(Piece* neighbor, qreal xPos, qreal yPos)
	: piece(neighbor)
	, relativeXPos(xPos)
	, relativeYPos(yPos)
{
}

Palapeli::Piece::Piece(const QPixmap& pixmap, Palapeli::Scene* scene, int width, int height)
	: QGraphicsPixmapItem()
	, m_width(width)
	, m_height(height)
	, m_scene(scene)
	, m_part(0)
	, m_moving(false)
{
	setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
	setPixmap(pixmap);
	setOffset(0.0, 0.0);

	m_scene->addItem(this);
	QRectF pieceRect = sceneBoundingRect();
	scale(width / pieceRect.width(), height / pieceRect.height());
}

Palapeli::Piece::~Piece()
{
}

int Palapeli::Piece::width() const
{
	return m_width;
}

int Palapeli::Piece::height() const
{
	return m_height;
}

Palapeli::Part* Palapeli::Piece::part() const
{
	return m_part;
}

void Palapeli::Piece::addNeighbor(Piece* piece, qreal xDiff, qreal yDiff)
{
	m_neighbors << Palapeli::Piece::NeighborInfo(piece, xDiff, yDiff);
}

void Palapeli::Piece::setPart(Palapeli::Part* part)
{
	m_part = part;
}

void Palapeli::Piece::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if (event->button() & Qt::LeftButton)
		m_moving = true;
}

void Palapeli::Piece::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	if (m_moving)
	{
		const QPointF difference = event->scenePos() - event->lastScenePos();
		m_part->moveAllBy(difference.x(), difference.y());
	}
}

void Palapeli::Piece::mouseReleaseEvent(QGraphicsSceneMouseEvent*)
{
	m_part->searchConnections();
	m_moving = false;
}

#include "piece.moc"

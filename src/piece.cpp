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

Palapeli::Piece::Piece(const QPixmap& pixmap, Palapeli::Scene* scene, int xIndex, int yIndex, int width, int height, QGraphicsItem* parent)
	: QGraphicsPixmapItem(parent)
	, m_scene(scene)
	, m_part(0)
	, m_xIndex(xIndex)
	, m_yIndex(yIndex)
{
	setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);

	setPixmap(pixmap);
	m_scene->addItem(this);
	QRectF pieceRect = sceneBoundingRect();
	scale(width / pieceRect.width(), height / pieceRect.height());
}

Palapeli::Piece::~Piece()
{
}

void Palapeli::Piece::setPart(Palapeli::Part* part) //friend of Palapeli::Part
{
	//change ownership in internal data structures of Part
	if (m_part != 0)
		m_part->m_pieces.removeAll(this);
	if (part != 0)
		part->m_pieces << this;
	//setup my own relationships
	m_part = part;
	setParentItem(part);
}

Palapeli::Part* Palapeli::Piece::part() const
{
	return m_part;
}

int Palapeli::Piece::xIndex() const
{
	return m_xIndex;
}

int Palapeli::Piece::yIndex() const
{
	return m_yIndex;
}

void Palapeli::Piece::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	m_part->mouseMoveEvent(event);
}

void Palapeli::Piece::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	m_part->mousePressEvent(event);

}

void Palapeli::Piece::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	m_part->mouseReleaseEvent(event);

}
#include "piece.moc"

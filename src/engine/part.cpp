/***************************************************************************
 *   Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
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

#include "part.h"
#include "piece.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QSet>

Palapeli::Part::Part(Palapeli::Piece* piece)
{
	m_pieces << piece;
	piece->setParentItem(this);
	setFlag(QGraphicsItem::ItemIsMovable);
	setHandlesChildEvents(true);
	piece->setFlag(QGraphicsItem::ItemStacksBehindParent); //DEBUG
}

Palapeli::Part::~Part()
{
	qDeleteAll(m_pieces);
}

void Palapeli::Part::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	QGraphicsItem::mouseReleaseEvent(event); //handle dragging
	if (event->button() == Qt::LeftButton)
	{
		//move item to top
		static int zValue = 1;
		setZValue(zValue);
		++zValue;
	}
}

void Palapeli::Part::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	QGraphicsItem::mouseMoveEvent(event); //handle dragging
	if (event->buttons() & Qt::LeftButton)
		validatePosition();
}

void Palapeli::Part::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	QGraphicsItem::mouseReleaseEvent(event); //handle dragging
	if (event->button() == Qt::LeftButton)
	{
		//check for parts that can be merged with this part
		QSet<Palapeli::Part*> mergeParts;
		foreach (Palapeli::Piece* piece, m_pieces)
		{
			const QList<Palapeli::Piece*> mergeNeighbors = piece->connectableNeighbors(0.2);
			foreach (Palapeli::Piece* mergeNeighbor, mergeNeighbors)
				mergeParts << qgraphicsitem_cast<Palapeli::Part*>(mergeNeighbor->parentItem());
		}
		//merge any parts that are near this part
		foreach (Palapeli::Part* part, mergeParts)
		{
			//set position in such a way that the majority of the pieces do not move
			if (part->m_pieces.count() > m_pieces.count())
				setPos(part->pos());
			//insert all pieces of the other part into this part
			foreach (Palapeli::Piece* piece, part->m_pieces)
			{
				piece->setParentItem(this);
				m_pieces << piece;
			}
			//delete other part, but avoid that the pieces are deleted
			part->m_pieces.clear();
			delete part;
		}
		//update internal neighbor lists in the pieces
		foreach (Palapeli::Piece* piece, m_pieces)
			piece->updateNeighborsList();
		//make position valid again
		validatePosition();
		emit partMoved();
	}
}

void Palapeli::Part::validatePosition()
{
	//ensure that part stays inside scene rect
	const QRectF sr = scene()->sceneRect();
	const QRectF br = sceneTransform().mapRect(childrenBoundingRect()); //br = bounding rect
	if (!sr.contains(br))
	{
		QPointF pos = this->pos();
		if (br.left() < sr.left())
			pos.rx() += sr.left() - br.left();
		if (br.right() > sr.right())
			pos.rx() += sr.right() - br.right();
		if (br.top() < sr.top())
			pos.ry() += sr.top() - br.top();
		if (br.bottom() > sr.bottom())
			pos.ry() += sr.bottom() - br.bottom();
		setPos(pos);
	}
}

#include "part.moc"

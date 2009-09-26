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

void Palapeli::Part::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	QGraphicsItem::mouseReleaseEvent(event); //handle dragging
	if (event->button() == Qt::LeftButton)
	{
		//check for parts that can be merged with this part
		QSet<Palapeli::Part*> mergeParts;
		foreach (Palapeli::Piece* piece, m_pieces)
		{
			const QList<Palapeli::Piece*> mergeNeighbors = piece->connectableNeighbors(0.1);
			foreach (Palapeli::Piece* mergeNeighbor, mergeNeighbors)
				mergeParts << qgraphicsitem_cast<Palapeli::Part*>(mergeNeighbor->parentItem());
		}
		//merge any parts that are near this part
		foreach (Palapeli::Part* part, mergeParts)
		{
			foreach (Palapeli::Piece* piece, part->m_pieces)
			{
				piece->setParentItem(this);
				m_pieces << piece;
			}
			part->m_pieces.clear(); //avoid that the pieces are deleted in ~Part
			delete part;
		}
		//update internal neighbor lists in the pieces
		foreach (Palapeli::Piece* piece, m_pieces)
			piece->updateNeighborsList();
		emit partMoved();
	}
}

#include "part.moc"

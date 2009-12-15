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

#include "piece.h"
#include "part.h"

Palapeli::Piece::Piece(const QPixmap& pixmap, const QPointF& offset)
	: m_pixmapItem(new QGraphicsPixmapItem(pixmap, this))
{
	m_pixmapItem->setOffset(offset);
	setAcceptedMouseButtons(Qt::LeftButton);
	setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

void Palapeli::Piece::addNeighbor(Palapeli::Piece* piece)
{
	if (!m_missingNeighbors.contains(piece))
		m_missingNeighbors << piece;
}

Palapeli::Part* Palapeli::Piece::part() const
{
	return qgraphicsitem_cast<Palapeli::Part*>(parentItem());
}

QGraphicsPixmapItem* Palapeli::Piece::pixmapItem() const
{
	return m_pixmapItem;
}

QList<Palapeli::Piece*> Palapeli::Piece::connectableNeighbors(qreal snappingPrecision) const
{
	QList<Palapeli::Piece*> result;
	const QSizeF ownSize = m_pixmapItem->pixmap().size();
	foreach (Palapeli::Piece* neighbor, m_missingNeighbors)
	{
		const QSizeF neighborSize = neighbor->m_pixmapItem->pixmap().size();
		const QSizeF snappingSize = snappingPrecision * ownSize.expandedTo(neighborSize);
		const QPointF posDifference = parentItem()->pos() - neighbor->parentItem()->pos(); //parentItem() is a Part (do not use part() to save a cast)
		if (qAbs(posDifference.x()) <= snappingSize.width() && qAbs(posDifference.y()) <= snappingSize.height())
			result << neighbor;
	}
	return result;
}

void Palapeli::Piece::updateNeighborsList()
{
	QMutableListIterator<Palapeli::Piece*> iter(m_missingNeighbors);
	while (iter.hasNext())
		if (iter.next()->parentItem() == parentItem()) //parentItem is a Part (do not use part() to save a cast)
			iter.remove();
}

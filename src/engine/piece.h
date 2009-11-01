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

#ifndef PALAPELI_PIECE_H
#define PALAPELI_PIECE_H

#include "basics.h"

namespace Palapeli
{
	class Part;

	class Piece : public Palapeli::GraphicsObject<Palapeli::PieceUserType>
	{
		public:
			Piece(const QPixmap& pixmap, const QPointF& offset);

			void addNeighbor(Palapeli::Piece* piece);
			Palapeli::Part* part() const;
			QGraphicsPixmapItem* pixmapItem() const;
		protected:
			friend class Part;
			QList<Palapeli::Piece*> connectableNeighbors(qreal snappingPrecision) const;
			void updateNeighborsList();
		private:
			QGraphicsPixmapItem* m_pixmapItem;
			QList<Palapeli::Piece*> m_missingNeighbors;
	};
}

#endif // PALAPELI_PIECE_H

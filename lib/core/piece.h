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

#ifndef PALAPELI_PIECE_H
#define PALAPELI_PIECE_H

#include "../macros.h"

#include <QGraphicsPixmapItem>

namespace Palapeli
{

	class Part;

	class PALAPELIBASE_EXPORT Piece : public QGraphicsPixmapItem
	{
		public:
			Piece(const QPixmap& pixmap, const QRectF& positionInImage);
			static Piece* fromPixmapPair(const QPixmap& pixmap, const QPixmap& mask, const QRectF& positionInImage);

			QPointF positionInImage() const;
			QSizeF size() const;

			Part* part() const;
			void setPart(Part* part);

			void makePositionValid(QPointF& basePosition) const;
		protected:
			virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);
			virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
			virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
		private:
			Part* m_part;
			QRectF m_positionInImage;

			bool m_moving;
			QPointF m_grabPosition;
	};

}

#endif //PALAPELI_PIECE_H

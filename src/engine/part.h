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

#ifndef PALAPELI_PART_H
#define PALAPELI_PART_H

#include <QGraphicsItem>
#include <QObject>

namespace Palapeli
{
	class Piece;

	class Part : public QObject, public QGraphicsItem
	{
		Q_OBJECT
		public:
			Part(Palapeli::Piece* piece);
			virtual ~Part();

			//empty QGraphicsItem reimplementation
			virtual QRectF boundingRect() const { return QRectF(); }
			virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* = 0) {}

			//enable qgraphicsitem_cast
			enum { Type = QGraphicsItem::UserType + 2 }; //UserType + 1 == Palapeli::Piece
			virtual int type() const { return Type; }
		Q_SIGNALS:
			void partMoved();
		protected:
			virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);
			virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
			virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
			void validatePosition();
		private:
			QList<Palapeli::Piece*> m_pieces;
	};
}

#endif // PALAPELI_PART_H
/***************************************************************************
 *   Copyright 2008 Felix Lemke <lemke.felix@ages-skripte.org>
 *   Copyright 2008 Stefan Majewsky <majewsky@gmx.net>
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

#include "../macros.h"

#include <QGraphicsItem>
#include <QList>

namespace Palapeli
{

	class Engine;
	class Piece;

	class PALAPELIBASE_EXPORT Part : public QGraphicsItem
	{
		public:
			Part(Piece* piece, Engine* engine);
			~Part(); //TODO: delete

			QPointF basePosition() const;
			void setBasePosition(const QPointF& basePosition);
			//TODO: refactor out by porting piecerelation.cpp to QGraphicsItem builtins
			int pieceCount() const;
			Piece* pieceAt(int index) const;
			//TODO: refactor out by making this a QObject and connecting Engine to deleted slot?
			Engine* engine() const;

			void addPiece(Piece* piece);
			void removePiece(Piece* piece);

			void move(const QPointF& newBasePosition);
			void update();

			//empty QGraphicsItem implementation
			virtual QRectF boundingRect() const { return QRectF(); }
			virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* = 0) {}
		protected:
// 			virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);
// 			virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
		private:
			QList<Palapeli::Piece*> m_pieces;
			QPointF m_basePosition;
			Palapeli::Engine* m_engine;
	};

}

#endif //PALAPELI_PART_H

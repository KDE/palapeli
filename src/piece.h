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

#include "part.h"

#include <QGraphicsPixmapItem>
#include <QList>
#include <QObject>

namespace Palapeli
{

	class Scene;

	class Piece : public QObject, public QGraphicsPixmapItem
	{
		friend void Part::searchConnections();
		Q_OBJECT
		public:
			Piece(const QPixmap &pixmap, Scene* scene, int width, int height);
			~Piece();

			int width() const;
			int height() const;
			Part* part() const;

			void addNeighbor(Piece* piece, qreal xDiff, qreal yDiff);
			void setPart(Part* part);

			virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);
			virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
			void move(qreal dx, qreal dy);
			virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
		private:
			struct NeighborInfo
			{
				NeighborInfo(Piece* neighbor, qreal xPos, qreal yPos);
				Piece* piece;
				qreal relativeXPos;
				qreal relativeYPos;
			};

			int m_width, m_height;
			Scene* m_scene;
			Part* m_part;
			QList<NeighborInfo> m_neighbors;
			bool m_moving;
	};

}

#endif //PALAPELI_PIECE_H

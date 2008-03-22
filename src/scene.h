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

#ifndef PALAPELI_SCENE_H
#define PALAPELI_SCENE_H

#include <QGraphicsScene>

class QGraphicsRectItem;
class QImage;

namespace Palapeli
{

	class Part;
	class Piece;

	class Scene : public QGraphicsScene
	{
		Q_OBJECT
		public:
			Scene(int width, int height);
			~Scene();

			void loadImage(const QImage& image, int xPieces, int yPieces);
			void combineParts(Part* part1, Part* part2, qreal dx, qreal dy);

			Piece* topNeighbor(int xIndex, int yIndex);
			Piece* bottomNeighbor(int xIndex, int yIndex);
			Piece* leftNeighbor(int xIndex, int yIndex);
			Piece* rightNeighbor(int xIndex, int yIndex);
		private:
			int m_xPieces, m_yPieces;
			Piece*** m_pieces;
			QList<Part*> m_parts;

			QGraphicsRectItem* m_visualSceneBoundary;
	};

}

#endif //PALAPELI_SCENE_H

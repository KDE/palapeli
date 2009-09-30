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

#ifndef PALAPELI_SCENE_H
#define PALAPELI_SCENE_H

#include <QGraphicsScene>

namespace Palapeli
{
	class Part;
	class Piece;
	class Puzzle;

	class Scene : public QGraphicsScene
	{
		Q_OBJECT
		public:
			Scene(QObject* parent = 0);
			void loadPuzzle(Palapeli::Puzzle* puzzle);
		public Q_SLOTS:
			void restartPuzzle();
		Q_SIGNALS:
			void reportProgress(int pieceCount, int partCount);
		private Q_SLOTS:
			void partDestroyed(QObject* object);
			void partMoved();
		private:
			QString m_identifier;
			Palapeli::Puzzle* m_puzzle;
			QList<Palapeli::Piece*> m_pieces;
			QList<Palapeli::Part*> m_parts;
	};
}

#endif // PALAPELI_SCENE_H

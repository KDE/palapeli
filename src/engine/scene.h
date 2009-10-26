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

#include <QFutureWatcher>
#include <QGraphicsScene>
#include <QMap>
class QModelIndex;
#include <QPointer>

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

			bool isConstrained() const;
		public Q_SLOTS:
			void loadPuzzle(const QModelIndex& index);
			void restartPuzzle();
			void setConstrained(bool constrained);
		Q_SIGNALS:
			void constrainedChanged(bool constrained);
			void reportProgress(int pieceCount, int partCount);
		private Q_SLOTS:
			void partDestroyed(QObject* object);
			void partMoved();
			//loading steps
			void startLoading();
			void continueLoading();
			void loadNextPart();
			void finishLoading();
		private:
			void loadPuzzleInternal();

			//behavioral parameteres
			bool m_constrained;
			//game parameters
			QString m_identifier;
			QPointer<Palapeli::Puzzle> m_puzzle;
			QMap<int, Palapeli::Piece*> m_pieces;
			QList<Palapeli::Part*> m_parts;
			//some stuff needed for loading puzzles
			bool m_loadingPuzzle;
			QFutureWatcher<bool> m_metadataLoader;
	};
}

#endif // PALAPELI_SCENE_H

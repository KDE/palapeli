/***************************************************************************
 *   Copyright 2009, 2010 Stefan Majewsky <majewsky@gmx.net>
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
	class ConstraintVisualizer;
	class Piece;
	class Puzzle;

	class Scene : public QGraphicsScene
	{
		Q_OBJECT
		public:
			Scene(QObject* parent = 0);

			void addPiece(Palapeli::Piece* piece);
			bool isConstrained() const;
			QRectF piecesBoundingRect() const;

			void validatePiecePosition(Palapeli::Piece* piece);
			void mergeLoadedPieces();
			const QSizeF& pieceAreaSize() {
				if (! m_pieceAreaSize.isValid()) {
					calculatePieceAreaSize();
				}
				return m_pieceAreaSize;
			}
			QList<Palapeli::Piece*> pieces() { return m_pieces; }
			void clearPieces();

			// IDW TODO - Making this public is a temporary fix?
			void calculatePieceAreaSize();
			// IDW TODO - DELETE?
			void startPuzzle() { emit puzzleStarted(); }

		public Q_SLOTS:
			void setConstrained(bool constrained);
		Q_SIGNALS:
			// IDW TODO - What is constrainedChanged() used for?
			void constrainedChanged(bool constrained);
			void puzzleStarted(); // IDW DELETE.
			void saveMove(int reduction);
		private Q_SLOTS:
			void pieceMoved(bool finished);
			void pieceInstanceTransaction(const QList<Palapeli::Piece*>& deletedPieces, const QList<Palapeli::Piece*>& createdPieces);
		private:
			void searchConnections(
					const QList<Palapeli::Piece*>& pieces,
					const bool animatedMerging = true);
			//behavior parameters
			bool m_constrained;
			Palapeli::ConstraintVisualizer* m_constraintVisualizer;
			//game parameters
			Palapeli::Puzzle* m_puzzle;
			QList<Palapeli::Piece*> m_pieces;
			int m_atomicPieceCount;
			QSizeF m_pieceAreaSize;
	};
}

#endif // PALAPELI_SCENE_H

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

	/**
	 * This class holds the puzzle pieces and boundary (constraint) of a
	 * Palapeli scene, which can be either a piece-holder or the main
	 * puzzle table. The scene also handles adding and removing pieces,
	 * moving pieces, merging (or joining) pieces, arranging pieces into
	 * a grid and signalling changes in the state of the puzzle and its
	 * pieces, wherever they may be.
	 */

	class Scene : public QGraphicsScene
	{
		Q_OBJECT
		public:
			Scene(QObject* parent = 0);

			void addPieceToList(Palapeli::Piece* piece);
			void addPieceItemsToScene();
			bool isConstrained() const;
			QRectF extPiecesBoundingRect() const;
			void setMinGrid(const int minGrid);
			QRectF piecesBoundingRect() const;
			qreal margin() { return m_margin; }
			qreal handleWidth() { return m_handleWidth; }
			void addMargin(const qreal handleWidth,
				       const qreal spacer);

			void validatePiecePosition(Palapeli::Piece* piece);
			void mergeLoadedPieces();
			const QSizeF& pieceAreaSize()
					{ return m_pieceAreaSize; }
			void setPieceAreaSize(const QSizeF& pieceAreaSize)
					{ m_pieceAreaSize = pieceAreaSize; }
			QList<Palapeli::Piece*> pieces() { return m_pieces; }

			void dispatchPieces(const QList<Palapeli::Piece*> pcs);
			void clearPieces();

			void initializeGrid(const QPointF& gridTopLeft);
			void addToGrid(Piece* piece);

		public Q_SLOTS:
			void setConstrained(bool constrained);
		Q_SIGNALS:
			void constrainedChanged(bool constrained);
			void saveMove(int reduction);
		private Q_SLOTS:
			void pieceMoved(bool finished);
			void pieceInstanceTransaction(
				const QList<Palapeli::Piece*>& deletedPieces,
				const QList<Palapeli::Piece*>& createdPieces);
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
			// Width of ConstraintVisualizer and space at edges.
			qreal m_margin;
			qreal m_handleWidth;

			QPointF m_gridTopLeft;
			QSizeF m_gridSpacing;
			int m_gridRank;
			int m_gridX;
			int m_gridY;
			// Scene has at least m_minGrid*m_minGrid piece-spaces.
			int m_minGrid;
	};
}

#endif // PALAPELI_SCENE_H

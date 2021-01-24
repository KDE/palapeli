/*
    SPDX-FileCopyrightText: 2009, 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
	 * a grid and signaling changes in the state of the puzzle and its
	 * pieces, wherever they may be.
	 */

	class Scene : public QGraphicsScene
	{
		Q_OBJECT
		friend class GamePlay;
		public:
			explicit Scene(QObject* parent = nullptr);

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

			void dispatchPieces(const QList<Palapeli::Piece*> &pcs);
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

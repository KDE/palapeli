/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>
    SPDX-FileCopyrightText: 2010 Johannes Löhnert <loehnert.kde@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELI_PIECE_H
#define PALAPELI_PIECE_H

#include "basics.h"
#include "piecevisuals.h"

class QPropertyAnimation;

namespace Palapeli
{
	class MovePieceInteractor;
	class PieceVisuals;

	class Piece : public Palapeli::GraphicsObject<Palapeli::PieceUserType>
	{
		Q_OBJECT
		Q_PROPERTY(qreal activeShadowOpacity READ activeShadowOpacity WRITE setActiveShadowOpacity)
		public:
			/// This constructor is used when the piece is loaded
			/// from the puzzle file as a single unjoined piece.
			explicit Piece(const QImage& pieceImage, const QPoint& offset);
			/// This constructor is used when several pieces are
			/// joined to create a new piece, either after a user's
			/// move or when such a piece is re-created at puzzle
			/// load time (without animation). The joined piece will
			/// be without a shadow, unless shadows are provided
			/// explicitly.
			explicit Piece(const Palapeli::PieceVisuals& pieceVisuals, const Palapeli::PieceVisuals& shadowVisuals = Palapeli::PieceVisuals(), const Palapeli::PieceVisuals& highlightVisuals = Palapeli::PieceVisuals());
			///This method will
			///\li create a shadow for this piece if there is none ATM.
			///\li apply the bevel map to the piece pixmap.
			///These operations need not be done until the piece is about to be shown.
			///\return whether something needed to be done
			bool completeVisuals();

			///Returns the bounding rect without shadows.
			QRectF bareBoundingRect() const;
			QRectF sceneBareBoundingRect() const;
			Palapeli::PieceVisuals pieceVisuals() const;
			bool hasShadow() const;
			bool hasHighlight() const;
			Palapeli::PieceVisuals shadowVisuals() const;
			Palapeli::PieceVisuals highlightVisuals() const;
			void createHighlight(const QSizeF& pieceAreaSize);

			bool isSelected() const;
			void setSelected(bool selected);
			void startClick();
			void endClick();
			///Returns the corresponding piece for an \a item found e.g. in QGraphicsScene::selectedItems(). This is different from a simple qgraphicsitem_cast because, internally, when you call setSelected(true) on a piece, a child item of this Piece is selected.
			///\return 0 if the given \a item does not belong to a Piece
			static Palapeli::Piece* fromSelectedItem(QGraphicsItem* item);

			///This method lets the piece remember which atomic pieces it represents. (Atomic pieces are what the scene creates when the puzzle is loaded.)
			void addRepresentedAtomicPieces(const QList<int>& representedAtomicPieces);
			QList<int> representedAtomicPieces() const;
			void addLogicalNeighbors(const QList<Palapeli::Piece*>& logicalNeighbors);
			QList<Palapeli::Piece*> logicalNeighbors() const;
			void addAtomicSize(const QSize& size);
			///This method returns the biggest size of an atomic piece contained in this piece.
			QSize atomicSize() const;

			///When piece instances have been replaced by other piece instances, this method can be used to update the internal data structures of their logical neighbors.
			void rewriteLogicalNeighbors(const QList<Palapeli::Piece*>& oldPieces, Palapeli::Piece* newPiece);
			///Call this when this piece instance has been replaced by another piece instance. This will also delete this instance.
			void announceReplaced(Palapeli::Piece* replacement);
			/// Place piece in a grid-cell, randomly or centered.
			void setPlace(const QPointF& topLeft, int x, int y,
					const QSizeF& area, bool random);
		Q_SIGNALS:
			void moved(bool finished);
			void replacedBy(Palapeli::Piece* newPiece);
		protected:
			friend class MovePieceInteractor;
			void beginMove();
			void doMove();
			void endMove();
		private Q_SLOTS:
			void pieceItemSelectedChanged(bool selected);
		private:
			void commonInit(const Palapeli::PieceVisuals& pieceVisuals);
			void createShadowItems(const Palapeli::PieceVisuals& shadowVisuals);
			qreal activeShadowOpacity() const;
			void setActiveShadowOpacity(qreal opacity);

			QGraphicsPixmapItem* m_pieceItem;
			QGraphicsPixmapItem* m_inactiveShadowItem;
			QGraphicsPixmapItem* m_activeShadowItem;
			QGraphicsPixmapItem* m_highlightItem; // IDW test.
			QPropertyAnimation* m_animator;
			QPoint m_offset; // IDW test.

			QList<int> m_representedAtomicPieces;
			QList<Palapeli::Piece*> m_logicalNeighbors;
			QSize m_atomicSize;
	};
}

#endif // PALAPELI_PIECE_H

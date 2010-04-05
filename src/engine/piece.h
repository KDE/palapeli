/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>
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

#include "basics.h"
#include "piecevisuals.h"

class QPropertyAnimation;

namespace Palapeli
{
	struct PieceVisuals;

	class Piece : public Palapeli::GraphicsObject<Palapeli::PieceUserType>
	{
		Q_OBJECT
		Q_PROPERTY(qreal activeShadowOpacity READ activeShadowOpacity WRITE setActiveShadowOpacity)
		public:
			///This constructor creates a piece without a shadow, unless a shadow is provided explicitly.
			Piece(const Palapeli::PieceVisuals& pieceVisuals, const Palapeli::PieceVisuals& shadowVisuals = Palapeli::PieceVisuals());
			///This method will create a shadow for this piece if there is none ATM.
			void createShadow();

			virtual QRectF boundingRect() const;
			///Returns the bounding rect without shadows.
			QRectF bareBoundingRect() const;
			QRectF sceneBareBoundingRect() const;
			Palapeli::PieceVisuals pieceVisuals() const;
			Palapeli::PieceVisuals shadowVisuals() const;

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
		Q_SIGNALS:
			void moved();
		protected:
			virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value);
			virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);
			virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
			virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
		private:
			void createShadowItems(const Palapeli::PieceVisuals& shadowVisuals);
			qreal activeShadowOpacity() const;
			void setActiveShadowOpacity(qreal opacity);

			QGraphicsPixmapItem* m_pieceItem;
			QGraphicsPixmapItem* m_inactiveShadowItem;
			QGraphicsPixmapItem* m_activeShadowItem;
			QPropertyAnimation* m_animator;

			QList<int> m_representedAtomicPieces;
			QList<Palapeli::Piece*> m_logicalNeighbors;
			QSize m_atomicSize;
	};
}

#endif // PALAPELI_PIECE_H

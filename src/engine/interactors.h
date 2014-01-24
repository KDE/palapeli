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

#ifndef PALAPELI_INTERACTORS_H
#define PALAPELI_INTERACTORS_H

#include "interactor.h"

#include <QGraphicsItem>

namespace Palapeli
{
	class Piece;

	//This interactor is assigned to LeftButton;NoModifier by default.
	//1. When you click on and drag a selected piece, all selected pieces are moved.
	//2. When you click on and drag an unselected piece, all other pieces are deselected, the clicked piece is selected and moved.
	class MovePieceInteractor : public QObject, public Palapeli::Interactor
	{
		Q_OBJECT
		public:
			MovePieceInteractor(QGraphicsView* view);
		protected:
			virtual bool startInteraction(const Palapeli::MouseEvent& event);
			virtual void continueInteraction(const Palapeli::MouseEvent& event);
			virtual void stopInteraction(const Palapeli::MouseEvent& event);
		protected Q_SLOTS:
			void pieceReplacedBy(Palapeli::Piece* replacement);
		private:
			Palapeli::Piece* findPieceForItem(QGraphicsItem* mouseInteractingItem) const;
			void determineSelectedItems(QGraphicsItem* clickedItem, Palapeli::Piece* clickedPiece);

			QList<Palapeli::Piece*> m_currentPieces;
			QPointF m_baseScenePosition, m_currentOffset;
			QPoint m_baseViewPosition;
			QList<QPointF> m_basePositions;
	};

	//This interactor is assigned to LeftButton;ControlModifier by default.
	//When you click on a piece, its selection state will be toggled.
	class SelectPieceInteractor : public Palapeli::Interactor
	{
		public:
			SelectPieceInteractor(QGraphicsView* view);
		protected:
			virtual bool startInteraction(const Palapeli::MouseEvent& event);
			virtual void stopInteraction(const Palapeli::MouseEvent& event);
		private:
			Palapeli::Piece* m_currentPiece;
	};

	//This interactor is assigned to RightButton;NoModifier by default.
	//Dragging will drag the viewport (only translations, no rotation or zooming).
	class MoveViewportInteractor : public Palapeli::Interactor
	{
		public:
			MoveViewportInteractor(QGraphicsView* view);
		protected:
			virtual bool startInteraction(const Palapeli::MouseEvent& event);
			virtual void continueInteraction(const Palapeli::MouseEvent& event);
		private:
			QPoint m_lastPos;
	};

	//This interactor is assigned to wheel:Vertical;NoModifier by default.
	//Turning the wheel will zoom the viewport.
	class ZoomViewportInteractor : public Palapeli::Interactor
	{
		public:
			ZoomViewportInteractor(QGraphicsView* view);
		protected:
			virtual void doInteraction(const Palapeli::WheelEvent& event);
	};

	//This interactor is assigned to nothing by default.
	//Turning the wheel will scroll the viewport either horizontally or vertically.
	class ScrollViewportInteractor : public Palapeli::Interactor
	{
		public:
			ScrollViewportInteractor(Qt::Orientation orientation, QGraphicsView* view);
		protected:
			virtual void doInteraction(const Palapeli::WheelEvent& event);
		private:
			Qt::Orientation m_orientation;
	};

	class RubberBandItem : public QGraphicsItem
	{
		public:
			RubberBandItem(QGraphicsItem* parent = 0);

			QRectF rect() const;
			void setRect(const QRectF& rect);

			virtual QRectF boundingRect() const;
			virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);
		private:
			QRectF m_rect;
	};

	//This interactor is assigned to LeftClick;NoModifier by default.
	//When you click on a free area of the view and drag the mouse, a rectangular rubberband will appear. All pieces inside the rubberband will be selected, all other pieces are deselected.
	class RubberBandInteractor : public Palapeli::Interactor
	{
		public:
			RubberBandInteractor(QGraphicsView* view);
			virtual ~RubberBandInteractor();
		protected:
			virtual void sceneChangeEvent(QGraphicsScene* oldScene, QGraphicsScene* newScene);
			virtual bool startInteraction(const Palapeli::MouseEvent& event);
			virtual void continueInteraction(const Palapeli::MouseEvent& event);
			virtual void stopInteraction(const Palapeli::MouseEvent& event);
		private:
			Palapeli::RubberBandItem* m_item;
			QPointF m_basePosition;
	};

	//This interactor is assigned to nothing by default.
	//When the interactor is triggered, the scene constraint is toggled.
	class ToggleConstraintInteractor : public Palapeli::Interactor
	{
		public:
			ToggleConstraintInteractor(QGraphicsView* view);
		protected:
			virtual bool startInteraction(const Palapeli::MouseEvent& event);
	};
}

#endif // PALAPELI_INTERACTORS_H

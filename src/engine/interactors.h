/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
			explicit MovePieceInteractor(QGraphicsView* view);
		protected:
			bool startInteraction(const Palapeli::MouseEvent& event) override;
			void continueInteraction(const Palapeli::MouseEvent& event) override;
			void stopInteraction(const Palapeli::MouseEvent& event) override;
		protected Q_SLOTS:
			void pieceReplacedBy(Palapeli::Piece* replacement);
		private:
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
			explicit SelectPieceInteractor(QGraphicsView* view);
		protected:
			bool startInteraction(const Palapeli::MouseEvent& event) override;
			void stopInteraction(const Palapeli::MouseEvent& event) override;
		private:
			Palapeli::Piece* m_currentPiece;
	};

	//This interactor is assigned to LeftButton;ShiftModifier by default.
	// 1. When you click on a piece in the puzzle table it is transferred
	//    immediately to the currently selected piece-holder.
	// 2. When you select one or more pieces on the puzzle table and click
	//    anywhere on the puzzle table, all the selected pieces are
	//    transferred immediately to the currently selected piece-holder.
	// 3. When you select one or more pieces in the currently selected
	//    piece-holder and then click anywhere on the puzzle table or in
	//    another piece-holder, all the selected pieces are transferred
	//    immediately to the place where you clicked.
	//
	//The transferred pieces are set selected when they arrive and are
	//arranged tidily in a grid pattern. In case 3, the top-left of the
	//grid is where you clicked.
	class TeleportPieceInteractor : public Palapeli::Interactor
	{
		public:
			explicit TeleportPieceInteractor(QGraphicsView* view);
		protected:
			bool startInteraction(const Palapeli::MouseEvent& event) override;
	};

	//This interactor is assigned to RightButton;NoModifier by default.
	//Dragging will drag the viewport (only translations, no rotation or zooming).
	class MoveViewportInteractor : public Palapeli::Interactor
	{
		public:
			explicit MoveViewportInteractor(QGraphicsView* view);
		protected:
			bool startInteraction(const Palapeli::MouseEvent& event) override;
			void continueInteraction(const Palapeli::MouseEvent& event) override;
		private:
			QPoint m_lastPos;
	};

	//This interactor is assigned to MidButton;NoModifier by default.
	//Clicking on a view will toggle it between close-up and distant views.
	class ToggleCloseUpInteractor : public Palapeli::Interactor
	{
		public:
			explicit ToggleCloseUpInteractor(QGraphicsView* view);
		protected:
			bool startInteraction(const Palapeli::MouseEvent& event) override;
	};

	//This interactor is assigned to wheel:Vertical;NoModifier by default.
	//Turning the wheel will zoom the viewport.
	class ZoomViewportInteractor : public Palapeli::Interactor
	{
		public:
			explicit ZoomViewportInteractor(QGraphicsView* view);
		protected:
			void doInteraction(const Palapeli::WheelEvent& event) override;
	};

	//This interactor is assigned to nothing by default.
	//Turning the wheel will scroll the viewport either horizontally or vertically.
	class ScrollViewportInteractor : public Palapeli::Interactor
	{
		public:
			ScrollViewportInteractor(Qt::Orientation orientation, QGraphicsView* view);
		protected:
			void doInteraction(const Palapeli::WheelEvent& event) override;
		private:
			Qt::Orientation m_orientation;
	};

	class RubberBandItem : public QGraphicsItem
	{
		public:
			explicit RubberBandItem(QGraphicsItem* parent = nullptr);

			QRectF rect() const;
			void setRect(const QRectF& rect);

			QRectF boundingRect() const override;
			void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
		private:
			QRectF m_rect;
	};

	//This interactor is assigned to LeftClick;NoModifier by default.
	//When you click on a free area of the view and drag the mouse, a rectangular rubberband will appear. All pieces inside the rubberband will be selected, all other pieces are deselected.
	class RubberBandInteractor : public Palapeli::Interactor
	{
		public:
			explicit RubberBandInteractor(QGraphicsView* view);
			~RubberBandInteractor() override;
		protected:
			void sceneChangeEvent(QGraphicsScene* oldScene, QGraphicsScene* newScene) override;
			bool startInteraction(const Palapeli::MouseEvent& event) override;
			void continueInteraction(const Palapeli::MouseEvent& event) override;
			void stopInteraction(const Palapeli::MouseEvent& event) override;
		private:
			Palapeli::RubberBandItem* m_item;
			QPointF m_basePosition;
	};

	//This interactor is assigned to nothing by default.
	//When the interactor is triggered, the scene constraint is toggled.
	class ToggleConstraintInteractor : public Palapeli::Interactor
	{
		public:
			explicit ToggleConstraintInteractor(QGraphicsView* view);
		protected:
			bool startInteraction(const Palapeli::MouseEvent& event) override;
	};
}

#endif // PALAPELI_INTERACTORS_H

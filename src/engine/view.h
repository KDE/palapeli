/*
    SPDX-FileCopyrightText: 2009, 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELI_VIEW_H
#define PALAPELI_VIEW_H

#include <QGraphicsView>

namespace Palapeli
{
	class InteractorManager;
	class Scene;
	class Piece;

	class View : public QGraphicsView
	{
		Q_OBJECT
		friend class GamePlay;
			Q_PROPERTY(QRectF viewportRect READ viewportRect WRITE setViewportRect)
		public:
			View();

			void puzzleStarted();
			Palapeli::InteractorManager* interactorManager() const;
			Palapeli::Scene* scene() const;

			QRectF viewportRect() const;
			void setViewportRect(const QRectF& viewportRect);
			void teleportPieces(Piece* piece, const QPointF& scPos);
			void toggleCloseUp();
			void setCloseUp(bool onOff);
			void handleNewPieceSelection();

			static const int MinimumZoomLevel;
			static const int MaximumZoomLevel;
		public Q_SLOTS:
			void logSceneChange(const QRectF &r); // IDW test.
			void setScene(Palapeli::Scene* scene);

			void moveViewportBy(const QPointF& sceneDelta);
			void zoomSliderInput(int level);
			void zoomIn();
			void zoomOut();
			void zoomBy(int delta); //delta = 0 -> no change, delta < 0 -> zoom out, delta > 0 -> zoom in
			void zoomTo(int level); //level = 100 -> actual size
		protected:
			void keyPressEvent(QKeyEvent* event) override;
			void keyReleaseEvent(QKeyEvent* event) override;
			void mouseMoveEvent(QMouseEvent* event) override;
			void mousePressEvent(QMouseEvent* event) override;
			void mouseReleaseEvent(QMouseEvent* event) override;
			void wheelEvent(QWheelEvent* event) override;
		Q_SIGNALS:
			void zoomLevelChanged(int level);
			void zoomAdjustable(bool adjustable);
			void teleport(Piece* p, const QPointF& scPos, View* v);
			void newPieceSelectionSeen(View* v);
		protected:
			virtual int calculateZoomRange(qreal distantScale,
							bool distantView);
			virtual qreal calculateCloseUpScale();
		private Q_SLOTS:
			void startVictoryAnimation();
			void adjustPointer();
		private:
			Palapeli::InteractorManager* m_interactorManager;
			Palapeli::Scene* m_scene;
			QPointF m_dragPrevPos;
			int m_zoomLevel;
			int m_closeUpLevel;
			int m_distantLevel;
			bool m_isCloseUp;
			qreal m_dZoom;
			qreal m_minScale;
			QPoint m_mousePos;
			QPointF m_scenePos;
			bool m_adjustPointer;
	};
}

#endif // PALAPELI_VIEW_H

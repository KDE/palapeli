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

#ifndef PALAPELI_VIEW_H
#define PALAPELI_VIEW_H

#include <QGraphicsView>

namespace Palapeli
{
	class ConstraintVisualizer;
	class InteractorManager;
	class Scene;
	class Piece;

	class View : public QGraphicsView
	{
		Q_OBJECT
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
			void logSceneChange(QRectF r); // IDW test.
			void setScene(Palapeli::Scene* scene);

			void moveViewportBy(const QPointF& sceneDelta);
			void zoomSliderInput(int level);
			void zoomIn();
			void zoomOut();
			void zoomBy(int delta); //delta = 0 -> no change, delta < 0 -> zoom out, delta > 0 -> zoom in
			void zoomTo(int level); //level = 100 -> actual size
		protected:
			virtual void keyPressEvent(QKeyEvent* event);
			virtual void keyReleaseEvent(QKeyEvent* event);
			virtual void mouseMoveEvent(QMouseEvent* event);
			virtual void mousePressEvent(QMouseEvent* event);
			virtual void mouseReleaseEvent(QMouseEvent* event);
			virtual void wheelEvent(QWheelEvent* event);
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

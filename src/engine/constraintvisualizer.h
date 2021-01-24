/*
    SPDX-FileCopyrightText: 2009, 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELI_CONSTRAINTVISUALIZER_H
#define PALAPELI_CONSTRAINTVISUALIZER_H

#include "basics.h"

class QPropertyAnimation;
#include <QVector>

namespace Palapeli
{
	class Scene;

	/* This class creates and manages a draggable frame around a Palapeli
	 * scene, which is used to expand or contract the scene. The principal
	 * Palapeli::Scene object is the puzzle table. When a puzzle is loaded,
	 * the puzzle table and other scenes have their frames (i.e. constraint
	 * visualizers) automatically set to surround the pieces closely.
	 *
	 * A scene can be locked by the user, via Scene::setConstrained(), in
	 * which case the frame and surrounding areas darken and the frame
	 * cannot be moved by pushing pieces against it, but the scene size
	 * can still be changed by dragging the frame with the mouse.
	 *
	 * The scene is then said to be "constrained" and its constraint
	 * visualizer object is "active" (ConstraintVisualizer::m_active true).
	 */
	class ConstraintVisualizer : public
		Palapeli::GraphicsObject<Palapeli::ConstraintVisualizerUserType>
	{
		Q_OBJECT
		public:
			explicit ConstraintVisualizer(Palapeli::Scene* scene);

			bool isActive() const;
			void start (const QRectF& sceneRect,
				    const int thickness);
			void stop();
		public Q_SLOTS:
			void setActive(bool active);
			void update(const QRectF& sceneRect);
		private:
			enum Side { LeftSide = 0, RightSide,
					TopSide, BottomSide, SideCount };
			enum HandlePosition { LeftHandle = 0, TopLeftHandle,
					TopHandle, TopRightHandle, RightHandle,
					BottomRightHandle, BottomHandle,
					BottomLeftHandle, HandleCount };

			Palapeli::Scene* m_scene;
			bool m_active;

			QVector<QGraphicsRectItem*> m_shadowItems, m_handleItems;
			QGraphicsPathItem* m_indicatorItem;
			QRectF m_sceneRect;
			QPropertyAnimation* m_animator;
			bool m_isStopped;
			qreal m_thickness;
	};
}

#endif // PALAPELI_CONSTRAINTVISUALIZER_H

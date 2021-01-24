/*
    SPDX-FileCopyrightText: 2009, 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "constraintvisualizer.h"
#include "scene.h"

#include <QPropertyAnimation>
#include <QCursor>
#include "palapeli_debug.h" // IDW test.

Palapeli::ConstraintVisualizer::ConstraintVisualizer(Palapeli::Scene* scene)
	: m_scene(scene)
	, m_active(false)
	, m_shadowItems(SideCount)
	, m_handleItems(HandleCount)
	, m_sceneRect(QRectF())
	, m_animator(new QPropertyAnimation(this, "opacity", this))
	, m_isStopped(true)
	, m_thickness(5.0)
{
	// All QGraphicsRectItems have null size until the first update().
	setOpacity(0.3);
	// Create shadow items. These are outside the puzzle table.
	QColor rectColor(Qt::black);
	// IDW test. rectColor.setAlpha(80);
	rectColor.setAlpha(40);				// Outer area is paler.
	for (int i = 0; i < SideCount; ++i)
	{
		m_shadowItems[i] = new QGraphicsRectItem(this);
		m_shadowItems[i]->setPen(Qt::NoPen);
		m_shadowItems[i]->setBrush(rectColor);
	}
	// Create handle items. These are the edges and corners of the table.
	// IDW test. rectColor.setAlpha(rectColor.alpha() / 2);
	rectColor.setAlpha(rectColor.alpha() * 2);	// Table edge is darker.
	Qt::CursorShape shapes[] = { Qt::SizeHorCursor, Qt::SizeFDiagCursor,
				     Qt::SizeVerCursor, Qt::SizeBDiagCursor };
	for (int i = 0; i < HandleCount; ++i)
	{
		m_handleItems[i] = new QGraphicsRectItem(this);
		m_handleItems[i]->setPen(Qt::NoPen);
		m_handleItems[i]->setBrush(rectColor);
		m_handleItems[i]->setCursor(shapes[i % 4]);
	}
	//more initialization
	QObject::setParent(scene); //delete myself automatically when the scene is destroyed
}

void Palapeli::ConstraintVisualizer::start (const QRectF& sceneRect,
					    const int thickness)
{
	// Puzzle loading nearly finished: add resize handles and shadow areas.
	if (!m_isStopped) {
		return;		// Duplicate call.
	}
	m_thickness = thickness;
	this->update(sceneRect);
	m_scene->addItem(this);
	setZValue(-1);

	// NOTE: The QueuedConnection is necessary because setSceneRect() sends
	// out the sceneRectChanged() signal before it disables automatic
	// growing of the scene rect. If the connection was direct, we could
	// thus enter an infinite loop when the constraint visualizer enlarges
	// itself in reaction to the changed sceneRect, thereby changing the
	// autogrowing sceneRect again.
	connect(m_scene, &Palapeli::Scene::sceneRectChanged, this, &ConstraintVisualizer::update, Qt::QueuedConnection);
	m_isStopped = false;
}

void Palapeli::ConstraintVisualizer::stop()
{
	if (m_isStopped) {
		return;		// Starting first loadPuzzle(): nothing to do.
	}
	m_scene->removeItem(this);
	disconnect(m_scene, &QGraphicsScene::sceneRectChanged, this, &ConstraintVisualizer::update);
	m_sceneRect = QRectF();
	m_isStopped = true;
}

bool Palapeli::ConstraintVisualizer::isActive() const
{
	return m_active;
}

void Palapeli::ConstraintVisualizer::setActive(bool active)
{
	if (m_active == active)
		return;
	m_active = active;
	const qreal targetOpacity = active ? 1.0 : 0.3;
	m_animator->setDuration(150 * qAbs(targetOpacity - opacity()));
	m_animator->setStartValue(opacity());
	m_animator->setEndValue(targetOpacity);
	m_animator->start();
}

void Palapeli::ConstraintVisualizer::update(const QRectF& sceneRect)
{
	if (m_sceneRect == sceneRect)
		return;
	// Make sure the ConstraintVisualizer stays outside the pieces' area.
	QRectF minimumRect = m_scene->extPiecesBoundingRect();
	m_sceneRect = sceneRect;
	if(!sceneRect.contains(minimumRect)) {
		// IDW TODO - Works and seems safe,
		//            but it may be better for interactor to check.
		m_sceneRect = minimumRect;
		m_scene->setSceneRect(minimumRect);
	}
	// Find a fictional viewport we want to cover (except for scene rect).
	const qreal viewportRectSizeFactor = 10;
	QRectF viewportRect = m_sceneRect;
	viewportRect.setSize(viewportRectSizeFactor * m_sceneRect.size());
	viewportRect.moveCenter(m_sceneRect.center());
	// The shadow areas are the areas outside the puzzle table.
	//adjust left shadow area
	QRectF itemRect = viewportRect;
	itemRect.setRight(m_sceneRect.left());
	m_shadowItems[LeftSide]->setRect(itemRect);
	//adjust right shadow area
	itemRect = viewportRect;
	itemRect.setLeft(m_sceneRect.right());
	m_shadowItems[RightSide]->setRect(itemRect);
	//adjust top shadow area
	itemRect = viewportRect;
	itemRect.setBottom(m_sceneRect.top());
	itemRect.setLeft(m_sceneRect.left()); //do not overlap left area...
	itemRect.setRight(m_sceneRect.right()); //..and right area
	m_shadowItems[TopSide]->setRect(itemRect);
	//adjust bottom shadow area
	itemRect = viewportRect;
	itemRect.setTop(m_sceneRect.bottom());
	itemRect.setLeft(m_sceneRect.left()); //same as above
	itemRect.setRight(m_sceneRect.right());
	m_shadowItems[BottomSide]->setRect(itemRect);
	//
	// The handles are the draggable borders of the puzzle table.
	//adjust edge handles
	// IDW test.QRectF handleRect(QPointF(), handleSize);
	QRectF handleRect(QPointF(), QSizeF(m_thickness, m_thickness));
	handleRect.moveTopLeft(m_sceneRect.topLeft());
	m_handleItems[TopLeftHandle]->setRect(handleRect);
	handleRect.moveTopRight(m_sceneRect.topRight());
	m_handleItems[TopRightHandle]->setRect(handleRect);
	handleRect.moveBottomLeft(m_sceneRect.bottomLeft());
	m_handleItems[BottomLeftHandle]->setRect(handleRect);
	handleRect.moveBottomRight(m_sceneRect.bottomRight());
	m_handleItems[BottomRightHandle]->setRect(handleRect);
	//adjust top/bottom handles
	// IDW test. handleRect.setSize(QSizeF(m_sceneRect.width() - 2 * handleSize.width(), handleSize.height()));
	handleRect.setSize(QSizeF(m_sceneRect.width() - 2 * m_thickness,
				m_thickness));
	handleRect.moveCenter(m_sceneRect.center());
	handleRect.moveTop(m_sceneRect.top());
	m_handleItems[TopHandle]->setRect(handleRect);
	handleRect.moveBottom(m_sceneRect.bottom());
	m_handleItems[BottomHandle]->setRect(handleRect);
	//adjust left/right handles
	// IDW test. handleRect.setSize(QSizeF(handleSize.width(), m_sceneRect.height() - 2 * handleSize.height()));
	handleRect.setSize(QSizeF(m_thickness,
				m_sceneRect.height() - 2 * m_thickness));
	handleRect.moveCenter(m_sceneRect.center());
	handleRect.moveLeft(m_sceneRect.left());
	m_handleItems[LeftHandle]->setRect(handleRect);
	handleRect.moveRight(m_sceneRect.right());
	m_handleItems[RightHandle]->setRect(handleRect);
}



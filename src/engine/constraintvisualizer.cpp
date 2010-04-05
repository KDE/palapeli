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

#include "constraintvisualizer.h"
#include "constraintvisualizer_p.h"
#include "scene.h"

#include <QGraphicsSceneMouseEvent>

//BEGIN Palapeli::CvHandleItem

Palapeli::CvHandleItem::CvHandleItem(const QCursor& cursor, const QColor& baseColor, QGraphicsItem* parent)
	: QGraphicsRectItem(QRectF(), parent)
	, m_baseColor(baseColor)
	, m_opacity(0)
	, m_animator(new QPropertyAnimation(this, "opacity", this))
{
	setPen(Qt::NoPen);
	setOpacity(0.01); //not visible in the beginning (HACK: QGV is overly clever and won't deliver any events to opacity=0.0 items)
	setCursor(cursor);
	setAcceptHoverEvents(true); //we need these for the animated show/hide
	setAcceptedMouseButtons(Qt::LeftButton); //this is for dragging
	setFlag(QGraphicsItem::ItemIsSelectable); //see CvHandleItem::itemChange
}

qreal Palapeli::CvHandleItem::opacity() const
{
	return m_opacity;
}

void Palapeli::CvHandleItem::setOpacity(qreal opacity)
{
	if (m_opacity == opacity)
		return;
	m_opacity = opacity;
	QColor brushColor(m_baseColor);
	brushColor.setAlpha(m_baseColor.alpha() * opacity);
	setBrush(brushColor);
}

void Palapeli::CvHandleItem::setOpacityAnimated(qreal targetOpacity)
{
	if (m_opacity == targetOpacity)
		return;
	m_animator->setDuration(150 * qAbs(targetOpacity - m_opacity));
	m_animator->setStartValue(m_opacity);
	m_animator->setEndValue(targetOpacity);
	m_animator->start();
}

QVariant Palapeli::CvHandleItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
	//HACK: The MovePieceInteractor does not propagate mouse events to non-selectable items. We therefore have the ItemIsSelectable flag set in this item, but deny any activation through this method.
	if (change == ItemSelectedChange)
		return qVariantFromValue(false);
	return QGraphicsRectItem::itemChange(change, value);
}

void Palapeli::CvHandleItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
	Q_UNUSED(event)
	setOpacityAnimated(1);
}

void Palapeli::CvHandleItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
	Q_UNUSED(event)
	setOpacityAnimated(0.01); //HACK: QGV is overly clever and won't deliver any events to opacity=0.0 items
}

void Palapeli::CvHandleItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	//let parent handle this type of event
	Palapeli::ConstraintVisualizer* cv = qgraphicsitem_cast<Palapeli::ConstraintVisualizer*>(parentItem());
	cv->mouseMoveEvent(event);
}

void Palapeli::CvHandleItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	//let parent handle this type of event
	Palapeli::ConstraintVisualizer* cv = qgraphicsitem_cast<Palapeli::ConstraintVisualizer*>(parentItem());
	cv->mousePressEvent(event);
}

//END Palapeli::CvHandleItem

Palapeli::ConstraintVisualizer::ConstraintVisualizer(Palapeli::Scene* scene)
	: m_scene(scene)
	, m_active(false)
	, m_shadowItems(SideCount)
	, m_handleItems(HandleCount)
	, m_handleExtent(0.0)
	, m_animator(new QPropertyAnimation(this, "opacity", this))
{
	setOpacity(0.2);
	//create gray items (with null size!)
	QColor rectColor(Qt::black);
	rectColor.setAlpha(80);
	for (int i = 0; i < SideCount; ++i)
	{
		m_shadowItems[i] = new QGraphicsRectItem(QRect(), this);
		m_shadowItems[i]->setPen(Qt::NoPen);
		m_shadowItems[i]->setBrush(rectColor);
	}
	rectColor.setAlpha(2 * rectColor.alpha());
	Qt::CursorShape shapes[] = { Qt::SizeHorCursor, Qt::SizeFDiagCursor, Qt::SizeVerCursor, Qt::SizeBDiagCursor };
	for (int i = 0; i < HandleCount; ++i)
		m_handleItems[i] = new Palapeli::CvHandleItem(shapes[i % 4], rectColor, this);
	//more initialization
	QObject::setParent(scene); //delete myself automatically when the scene is destroyed
	scene->addItem(this);
	setZValue(-1);
	connect(scene, SIGNAL(sceneRectChanged(const QRectF&)), this, SLOT(update(const QRectF&)));
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
	const qreal targetOpacity = active ? 1.0 : 0.2;
	m_animator->setDuration(150 * qAbs(targetOpacity - opacity()));
	m_animator->setStartValue(opacity());
	m_animator->setEndValue(targetOpacity);
	m_animator->start();
}

void Palapeli::ConstraintVisualizer::update(const QRectF& sceneRect)
{
	if (m_sceneRect == sceneRect)
		return;
	m_sceneRect = sceneRect;
	m_handleExtent = qMin(sceneRect.width(), sceneRect.height()) / 20;
	//find a fictional viewport rect which we want to cover (except for the contained scene rect)
	const qreal viewportRectSizeFactor = 10;
	QRectF viewportRect = sceneRect;
	viewportRect.setSize(viewportRectSizeFactor * sceneRect.size());
	viewportRect.moveCenter(sceneRect.center());
	//adjust left shadow area
	QRectF itemRect = viewportRect;
	itemRect.setRight(sceneRect.left());
	m_shadowItems[LeftSide]->setRect(itemRect);
	//adjust right shadow area
	itemRect = viewportRect;
	itemRect.setLeft(sceneRect.right());
	m_shadowItems[RightSide]->setRect(itemRect);
	//adjust top shadow area
	itemRect = viewportRect;
	itemRect.setBottom(sceneRect.top());
	itemRect.setLeft(sceneRect.left()); //do not overlap left area...
	itemRect.setRight(sceneRect.right()); //..and right area
	m_shadowItems[TopSide]->setRect(itemRect);
	//adjust bottom shadow area
	itemRect = viewportRect;
	itemRect.setTop(sceneRect.bottom());
	itemRect.setLeft(sceneRect.left()); //same as above
	itemRect.setRight(sceneRect.right());
	m_shadowItems[BottomSide]->setRect(itemRect);
	//adjust edge handles
	QRectF handleRect(QPointF(), QSizeF(m_handleExtent, m_handleExtent));
	handleRect.moveTopLeft(sceneRect.topLeft());
	m_handleItems[TopLeftHandle]->setRect(handleRect);
	handleRect.moveTopRight(sceneRect.topRight());
	m_handleItems[TopRightHandle]->setRect(handleRect);
	handleRect.moveBottomLeft(sceneRect.bottomLeft());
	m_handleItems[BottomLeftHandle]->setRect(handleRect);
	handleRect.moveBottomRight(sceneRect.bottomRight());
	m_handleItems[BottomRightHandle]->setRect(handleRect);
	//adjust top/bottom handles
	handleRect.setSize(QSizeF(sceneRect.width() - 2 * m_handleExtent, m_handleExtent));
	handleRect.moveCenter(sceneRect.center());
	handleRect.moveTop(sceneRect.top());
	m_handleItems[TopHandle]->setRect(handleRect);
	handleRect.moveBottom(sceneRect.bottom());
	m_handleItems[BottomHandle]->setRect(handleRect);
	//adjust left/right handles
	handleRect.setSize(QSizeF(m_handleExtent, sceneRect.height() - 2 * m_handleExtent));
	handleRect.moveCenter(sceneRect.center());
	handleRect.moveLeft(sceneRect.left());
	m_handleItems[LeftHandle]->setRect(handleRect);
	handleRect.moveRight(sceneRect.right());
	m_handleItems[RightHandle]->setRect(handleRect);
}

void Palapeli::ConstraintVisualizer::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	const QPointF pos = event->scenePos();
	if (m_sceneRect.contains(pos))
	{
		event->accept();
		m_lastScreenPos = event->screenPos();
	}
	else
	{
		event->ignore(); //do not react to clicks on the shadow items
		return;
	}
	//determine which coordinates can be moved
	m_draggingSides.clear();
	if (m_sceneRect.left() >= pos.x() - m_handleExtent)
		m_draggingSides << LeftSide;
	else if (m_sceneRect.right() <= pos.x() + m_handleExtent)
		m_draggingSides << RightSide;
	if (m_sceneRect.top() >= pos.y() - m_handleExtent)
		m_draggingSides << TopSide;
	else if (m_sceneRect.bottom() <= pos.y() + m_handleExtent)
		m_draggingSides << BottomSide;
}

void Palapeli::ConstraintVisualizer::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	event->accept();
	//prevent infinite loops (When the mouse reaches the end of the scene, the following will happen: 1. The ConstraintVisualizer enlarges the scene rect. 2. The QGraphicsView moves its viewport to accommodate the new scene rect. 3. The QGraphicsView might notice that the scene mouse position has changed, and fire a new mouseMoveEvent. 4. Repeat with step 1.)
	if (m_lastScreenPos == event->screenPos())
		return;
	m_lastScreenPos = event->screenPos();
	//modify scene rect
	QRectF newSceneRect = m_sceneRect;
	const QPointF pos = event->scenePos();
	const QPointF posDiff = event->scenePos() - event->lastScenePos();
	if (m_draggingSides.contains(LeftSide))
		newSceneRect.setLeft(m_sceneRect.left() + posDiff.x());
	if (m_draggingSides.contains(RightSide))
		newSceneRect.setRight(m_sceneRect.right() + posDiff.x());
	if (m_draggingSides.contains(TopSide))
		newSceneRect.setTop(m_sceneRect.top() + posDiff.y());
	if (m_draggingSides.contains(BottomSide))
		newSceneRect.setBottom(m_sceneRect.bottom() + posDiff.y());
	newSceneRect |= m_scene->piecesBoundingRect();
	m_scene->setSceneRect(newSceneRect);
}

#include "constraintvisualizer.moc"
#include "constraintvisualizer_p.moc"

/***************************************************************************
 *   Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
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

#include <QEvent>
#include <QGraphicsRectItem>
#include <QGraphicsView>
#include <QPropertyAnimation>

Palapeli::ConstraintVisualizer::ConstraintVisualizer(QGraphicsView* view)
	: m_view(view)
	, m_active(false)
	, m_items(PositionCount)
	, m_animator(new QPropertyAnimation(this, "opacity", this))
{
	setOpacity(0.2);
	//create gray items (with null size!)
	QColor rectColor(Qt::black);
	rectColor.setAlpha(80);
	for (int i = 0; i < PositionCount; ++i)
	{
		m_items[i] = new QGraphicsRectItem(QRect(), this);
		m_items[i]->setPen(Qt::NoPen);
		m_items[i]->setBrush(rectColor);
	}
	//more initialization
	QObject::setParent(view); //delete myself automatically when the view is destroyed
	view->viewport()->installEventFilter(this);
	view->scene()->addItem(this);
	connect(view->scene(), SIGNAL(sceneRectChanged(const QRectF&)), this, SLOT(update()));
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
	m_animator->setDuration(200 * qAbs(targetOpacity - opacity()));
	m_animator->setStartValue(opacity());
	m_animator->setEndValue(targetOpacity);
	m_animator->start();
}

bool Palapeli::ConstraintVisualizer::eventFilter(QObject* sender, QEvent* event)
{
	if (sender == m_view->viewport() && event->type() == QEvent::Paint)
		update();
	return QObject::eventFilter(sender, event);
}

void Palapeli::ConstraintVisualizer::update()
{
	//find viewport rect
	const QRectF viewportRect = m_view->mapToScene(m_view->rect()).boundingRect();
	const QRectF sceneRect = m_view->sceneRect();
	if (m_viewportRect == viewportRect)
		return;
	m_viewportRect = viewportRect;
	//adjust left gray area
	QRectF itemRect = viewportRect;
	itemRect.setRight(sceneRect.left());
	m_items[LeftPos]->setRect(itemRect);
	//adjust right gray area
	itemRect = viewportRect;
	itemRect.setLeft(sceneRect.right());
	m_items[RightPos]->setRect(itemRect);
	//adjust top gray area
	itemRect = viewportRect;
	itemRect.setBottom(sceneRect.top());
	itemRect.setLeft(sceneRect.left()); //do not overlap left area...
	itemRect.setRight(sceneRect.right()); //..and right area
	m_items[TopPos]->setRect(itemRect);
	//adjust bottom gray area
	itemRect = viewportRect;
	itemRect.setTop(sceneRect.bottom());
	itemRect.setLeft(sceneRect.left()); //same as above
	itemRect.setRight(sceneRect.right());
	m_items[BottomPos]->setRect(itemRect);
}

#include "constraintvisualizer.moc"

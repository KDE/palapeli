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

#include "inaccessibleareashelper.h"

#include <QEvent>
#include <QGraphicsRectItem>
#include <QGraphicsView>

Palapeli::InaccessibleAreasHelper::InaccessibleAreasHelper(QGraphicsView* view)
	: m_view(view)
	, m_items(PositionCount)
{
	//create gray items (with null size!)
	QColor rectColor(Qt::black);
	rectColor.setAlpha(42);
	const QPen pen(Qt::NoPen);
	const QBrush brush(rectColor);
	for (int i = 0; i < PositionCount; ++i)
		m_items[i] = view->scene()->addRect(QRect(), pen, brush);
	//more initialization
	QObject::setParent(view); //delete myself automatically when the view is destroyed
	view->viewport()->installEventFilter(this);
}

bool Palapeli::InaccessibleAreasHelper::eventFilter(QObject* sender, QEvent* event)
{
	if (sender == m_view->viewport() && event->type() == QEvent::Paint)
		update();
	return QObject::eventFilter(sender, event);
}

void Palapeli::InaccessibleAreasHelper::update()
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
	m_items[TopPos]->setRect(itemRect);
	//adjust bottom gray area
	itemRect = viewportRect;
	itemRect.setTop(sceneRect.bottom());
	m_items[BottomPos]->setRect(itemRect);
}

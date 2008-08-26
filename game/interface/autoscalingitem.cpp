/***************************************************************************
 *   Copyright (C) 2008 Stefan Majewsky <majewsky@gmx.net>
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

#include "autoscalingitem.h"

#include <QApplication>
#include <QGraphicsView>

Palapeli::AutoscalingItem::AutoscalingItem(QGraphicsView* view)
	: QGraphicsItem(0)
	, m_currentScalingFactor(1.0, 1.0)
	, m_view(view)
{
	updateScalingFactor();
	m_view->scene()->addItem(this);
	connect(m_view, SIGNAL(viewportMoved()), this, SLOT(updateScalingFactor()));
	connect(m_view, SIGNAL(viewportScaled()), this, SLOT(updateScalingFactor()));
}

void Palapeli::AutoscalingItem::updateScalingFactor()
{
	//measure scaling factor
	const qreal distance = 100.0;
	const QPointF point1 = m_view->mapToScene(0.0, 0.0);
	const QPointF point2 = m_view->mapToScene(distance, distance);
	const QPointF newScalingFactor((point2.x() - point1.x()) / distance, (point2.y() - point1.y()) / distance);
	const QPointF basePoint = QApplication::isLeftToRight() ? point1 : m_view->mapToScene(m_view->width() - 2.0 * m_view->frameWidth(), 0.0);
	//adapt to and save scaling factor
	setPos(basePoint); //set base position of coordinate system to top left edge of viewport
	scale(newScalingFactor.x() / m_currentScalingFactor.x(), newScalingFactor.y() / m_currentScalingFactor.y());
	m_currentScalingFactor = newScalingFactor;
}

#include "autoscalingitem.moc"

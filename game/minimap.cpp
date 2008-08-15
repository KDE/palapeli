/***************************************************************************
 *   Copyright (C) 2008 Felix Lemke <lemke.felix@ages-skripte.org>
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

#include "minimap.h"
#include "manager.h"
#include "piece.h"
#include "piecerelation.h"
#include "settings.h"
#include "view.h"

#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>

Palapeli::Minimap::Minimap(QWidget* parent)
	: QWidget(parent)
	, m_draggingViewport(false)
	, m_viewportWasDragged(false)
	, m_qualityLevel(Settings::minimapQuality())
{
	setBackgroundRole(QPalette::Window);
	setMinimumSize(150, 150);
}

void Palapeli::Minimap::setQualityLevel(int level)
{
	if (m_qualityLevel == level)
		return;
	m_qualityLevel = level;
	update();
	Settings::setMinimapQuality(level);
}

QRectF Palapeli::Minimap::viewport() const
{
	const QRect viewRect(QPoint(0, 0), ppMgr()->view()->viewport()->size());
	return ppMgr()->view()->mapToScene(viewRect).boundingRect();
}

QPointF Palapeli::Minimap::widgetToScene(const QPointF& point) const
{
	const QSizeF sceneSize = ppMgr()->view()->realScene()->sceneRect().size();
	const qreal sceneScalingFactor = qMin(width() / sceneSize.width(), height() / sceneSize.height());
	return QPointF(point.x() / sceneScalingFactor, point.y() / sceneScalingFactor);
}

void Palapeli::Minimap::moveViewport(const QPointF& widgetTo, const QPointF& widgetFrom)
{
	Palapeli::View* view = ppMgr()->view();
	//translate range of sliders in their coordinates and scene coordinates
	const qreal sliderMinimumX = view->horizontalScrollBar()->minimum();
	const qreal sliderMaximumX = view->horizontalScrollBar()->maximum();
	const qreal sliderMinimumY = view->verticalScrollBar()->minimum();
	const qreal sliderMaximumY = view->verticalScrollBar()->maximum();
	//attention: slider does only move center, it stops when the viewport edges reach the scene bounds
	const QRectF sceneViewport = viewport();
	const qreal sceneMinimumX = sceneViewport.width() / 2.0;
	const qreal sceneMinimumY = sceneViewport.height() / 2.0;
	//how to translate scene to slider coordinates
	const QSizeF sceneSize = view->realScene()->sceneRect().size();
	const qreal scalingX = (sliderMaximumX - sliderMinimumX) / (sceneSize.width() - sceneViewport.width());
	const qreal scalingY = (sliderMaximumY - sliderMinimumY) / (sceneSize.height() - sceneViewport.height());
	//widgetFrom.isNull() -> move to widgetFrom, !widgetFrom.isNull() -> move by widgetTo - widgetFrom
	if (widgetFrom.isNull())
	{
		//find desired position in scene coordinates
		const QPointF scenePos = widgetToScene(widgetTo);
		//translate to slider coordinates
		view->horizontalScrollBar()->setValue((scenePos.x() - sceneMinimumX) * scalingX);
		view->verticalScrollBar()->setValue((scenePos.y() - sceneMinimumY) * scalingY);
	}
	else
	{
		//find move difference in scene coordinates
		const QPointF sceneDiff = widgetToScene(widgetTo - widgetFrom);
		//translate move to slider coordinates and add move to original slider values
		view->horizontalScrollBar()->setValue(view->horizontalScrollBar()->value() + sceneDiff.x() * scalingX);
		view->verticalScrollBar()->setValue(view->verticalScrollBar()->value() + sceneDiff.y() * scalingY);
	}
}

void Palapeli::Minimap::mousePressEvent(QMouseEvent* event)
{
	if (event->button() & Qt::LeftButton)
	{
		//if user did not click on view rectangle, move viewport to the click position
		if (!viewport().contains(widgetToScene(event->pos())))
			moveViewport(event->pos());
		//start dragging
		m_draggingViewport = true;
		m_viewportWasDragged = false;
		//save position for next event
		m_draggingPreviousPos = event->pos();
	}
}

void Palapeli::Minimap::mouseMoveEvent(QMouseEvent* event)
{
	if (m_draggingViewport)
	{
		moveViewport(event->pos(), m_draggingPreviousPos);
		//save current position for next drag step
		m_draggingPreviousPos = event->pos();
		m_viewportWasDragged = true;
	}
}

void Palapeli::Minimap::mouseReleaseEvent(QMouseEvent* event)
{
	if (m_draggingViewport && !m_viewportWasDragged)
		//viewport was not dragged after mousePressEvent -> mouse did not move -> move viewport to this position
		moveViewport(event->pos());
	m_draggingViewport = m_viewportWasDragged = false;
}

void Palapeli::Minimap::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	//set painting metrics
	const QRectF sceneRect = ppMgr()->view()->realScene()->sceneRect();
	const QSizeF sceneSize = sceneRect.size();
	const qreal sceneWidth = sceneSize.width(), sceneHeight = sceneSize.height();
	const qreal scalingFactor = qMin(width() / sceneWidth, height() / sceneHeight);
	painter.scale(scalingFactor, scalingFactor);
	painter.setClipRect(sceneRect);

	//draw view rectangle
	painter.setBrush(palette().base());
	painter.drawRect(viewport());

	//draw piece positions
	QColor pieceColor = palette().highlight().color();
	pieceColor.setAlpha(m_qualityLevel == 1 ? 192 : 255);
	painter.setBrush(pieceColor);
	for (int i = 0; i < ppMgr()->pieceCount(); ++i)
		painter.drawRect(ppMgr()->pieceAt(i)->sceneBoundingRect());
}

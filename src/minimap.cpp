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
#include "piece.h"
#include "scene.h"
#include "view.h"

#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>

Palapeli::Minimap::Minimap()
	: QWidget()
	, m_view(0)
	, m_scene(0)
	, m_draggingViewport(false)
	, m_viewportWasDragged(false)
{
	setBackgroundRole(QPalette::Window);
	setMinimumSize(200, 200);
}

Palapeli::Minimap::~Minimap()
{
}

void Palapeli::Minimap::setView(View* view)
{
	m_view = view;
	m_scene = view->puzzleScene();
	connect(m_view, SIGNAL(viewportMoved()), this, SLOT(update()));
	connect(m_scene, SIGNAL(minimapNeedsUpdate()), this, SLOT(update()));
	update();
}

QPolygonF Palapeli::Minimap::viewport() const
{
	const QRect viewRect(0, 0, m_view->viewport()->width(), m_view->viewport()->height());
	return m_view->mapToScene(viewRect);
}

QPointF Palapeli::Minimap::widgetToScene(const QPointF& point) const
{
	const QSizeF sceneSize = m_scene->sceneRect().size();
	const qreal sceneScalingFactor = qMin(width() / sceneSize.width(), height() / sceneSize.height());
	return QPointF(point.x() / sceneScalingFactor, point.y() / sceneScalingFactor);
}

void Palapeli::Minimap::moveViewport(const QPointF& widgetTo, const QPointF& widgetFrom)
{
	//translate range of sliders in their coordinates and scene coordinates
	const qreal sliderMinimumX = m_view->horizontalScrollBar()->minimum();
	const qreal sliderMaximumX = m_view->horizontalScrollBar()->maximum();
	const qreal sliderMinimumY = m_view->verticalScrollBar()->minimum();
	const qreal sliderMaximumY = m_view->verticalScrollBar()->maximum();
	//attention: slider does only move center, it stops when the viewport edges reach the scene bounds
	const QRectF sceneViewport = viewport().boundingRect();
	const qreal sceneMinimumX = sceneViewport.width() / 2.0;
	const qreal sceneMinimumY = sceneViewport.height() / 2.0;
	//how to translate scene to slider coordinates
	const QSizeF sceneSize = m_scene->sceneRect().size();
	const qreal scalingX = (sliderMaximumX - sliderMinimumX) / (sceneSize.width() - sceneViewport.width());
	const qreal scalingY = (sliderMaximumY - sliderMinimumY) / (sceneSize.height() - sceneViewport.height());
	//widgetFrom.isNull() -> move to widgetFrom, !widgetFrom.isNull() -> move by widgetTo - widgetFrom
	if (widgetFrom.isNull())
	{
		//find desired position in scene coordinates
		const QPointF scenePos = widgetToScene(widgetTo);
		//translate to slider coordinates
		m_view->horizontalScrollBar()->setValue((scenePos.x() - sceneMinimumX) * scalingX);
		m_view->verticalScrollBar()->setValue((scenePos.y() - sceneMinimumY) * scalingY);
	}
	else
	{
		//find move difference in scene coordinates
		const QPointF sceneDiff = widgetToScene(widgetTo - widgetFrom);
		//translate move to slider coordinates and add move to original slider values
		m_view->horizontalScrollBar()->setValue(m_view->horizontalScrollBar()->value() + sceneDiff.x() * scalingX);
		m_view->verticalScrollBar()->setValue(m_view->verticalScrollBar()->value() + sceneDiff.y() * scalingY);
	}
}

void Palapeli::Minimap::mousePressEvent(QMouseEvent* event)
{
	if (event->button() & Qt::LeftButton)
	{
		//if user did not click on view rectangle, move viewport to the click position
		if (!viewport().boundingRect().contains(widgetToScene(event->pos())))
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

void Palapeli::Minimap::paintEvent(QPaintEvent*) //friend of Palapeli::Piece
{
	if (m_view)
	{
		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing);
		const QSizeF sceneSize = m_scene->sceneRect().size();
		const qreal sceneWidth = sceneSize.width(), sceneHeight = sceneSize.height();
		const qreal scalingFactor = qMin(width() / sceneWidth, height() / sceneHeight);
		painter.scale(scalingFactor, scalingFactor);
		painter.setClipRect(m_scene->sceneRect());
		const QPen pen(palette().highlight().color());
	
		//draw view rectangle
		painter.setBrush(palette().base());
		painter.drawPolygon(viewport());
		//draw piece positions
		QListIterator<Palapeli::Piece*> iterPieces = m_scene->pieces();
		QList<const Palapeli::Piece*> neighborsAlreadyConnected;
		while (iterPieces.hasNext())
		{
			const Palapeli::Piece* piece = iterPieces.next();
			const QRectF pieceRect = piece->sceneBoundingRect();
			const qreal pieceX = pieceRect.x(), pieceY = pieceRect.y();
			const qreal pieceWidth = pieceRect.width(), pieceHeight = pieceRect.height();
			qreal pieceCenterX = pieceX + 0.5 * pieceWidth, pieceCenterY = pieceY + 0.5 * pieceHeight; //may change!
			const bool isBeyondLeft = pieceCenterX <= 0, isBeyondRight = pieceCenterX >= sceneSize.width();
			const bool isAboveTop = pieceCenterY <= 0, isBelowBottom = pieceCenterY >= sceneSize.height();
			//out of range of minimap - draw arrow which points to piece position
			if (isAboveTop)
			{
				if (isBeyondLeft)
				{
					painter.drawLine(0, 0, 0.5 * pieceWidth, 0.5 * pieceHeight);
					painter.drawLine(0, 0, 0.25 * pieceWidth, 0);
					painter.drawLine(0, 0, 0, 0.25 * pieceHeight);
					pieceCenterX = 0;
					pieceCenterY = 0;
				}
				else if (isBeyondRight)
				{
					painter.drawLine(sceneWidth, 0, sceneWidth - 0.5 * pieceWidth, 0.5 * pieceHeight);
					painter.drawLine(sceneWidth, 0, sceneWidth - 0.25 * pieceWidth, 0);
					painter.drawLine(sceneWidth, 0, sceneWidth, 0.25 * pieceHeight);
					pieceCenterX = sceneWidth;
					pieceCenterY = 0;
				}
				else
				{
					painter.drawLine(pieceCenterX, 0, pieceCenterX, 0.5 * pieceHeight);
					painter.drawLine(pieceCenterX, 0, pieceCenterX - 0.25 * pieceWidth, 0.25 * pieceHeight);
					painter.drawLine(pieceCenterX, 0, pieceCenterX + 0.25 * pieceWidth, 0.25 * pieceHeight);
					pieceCenterY = 0;
				}
			}
			else if (isBelowBottom)
			{
				if (isBeyondLeft)
				{
					painter.drawLine(0, sceneHeight, 0.5 * pieceWidth, sceneHeight - 0.5 * pieceHeight);
					painter.drawLine(0, sceneHeight, 0.25 * pieceWidth, sceneHeight);
					painter.drawLine(0, sceneHeight, 0, sceneHeight - 0.25 * pieceHeight);
					pieceCenterX = 0;
					pieceCenterY = sceneHeight;
				}
				else if (isBeyondRight)
				{
					painter.drawLine(sceneWidth, sceneHeight, sceneWidth - 0.5 * pieceWidth, sceneHeight - 0.5 * pieceHeight);
					painter.drawLine(sceneWidth, sceneHeight, sceneWidth - 0.25 * pieceWidth, sceneHeight);
					painter.drawLine(sceneWidth, sceneHeight, sceneWidth, sceneHeight - 0.25 * pieceHeight);
					pieceCenterX = sceneWidth;
					pieceCenterY = sceneHeight;
				}
				else
				{
					painter.drawLine(pieceCenterX, sceneHeight, pieceCenterX, sceneHeight - 0.5 * pieceHeight);
					painter.drawLine(pieceCenterX, sceneHeight, pieceCenterX - 0.25 * pieceWidth, sceneHeight - 0.25 * pieceHeight);
					painter.drawLine(pieceCenterX, sceneHeight, pieceCenterX + 0.25 * pieceWidth, sceneHeight - 0.25 * pieceHeight);
					pieceCenterY = sceneHeight;
				}
			}
			else if (isBeyondLeft)
			{
				painter.drawLine(0, pieceCenterY, 0.5 * pieceWidth, pieceCenterY);
				painter.drawLine(0, pieceCenterY, 0.25 * pieceWidth, pieceCenterY - 0.25 * pieceHeight);
				painter.drawLine(0, pieceCenterY, 0.25 * pieceWidth, pieceCenterY + 0.25 * pieceHeight);
				pieceCenterX = 0;
			}
			else if (isBeyondRight)
			{
				painter.drawLine(sceneWidth, pieceCenterY, sceneWidth - 0.5 * pieceWidth, pieceCenterY);
				painter.drawLine(sceneWidth, pieceCenterY, sceneWidth - 0.25 * pieceWidth, pieceCenterY - 0.25 * pieceHeight);
				painter.drawLine(sceneWidth, pieceCenterY, sceneWidth - 0.25 * pieceWidth, pieceCenterY + 0.25 * pieceHeight);
				pieceCenterX = sceneWidth;
			}
			else
			{
				//draw cross at piece position
				painter.drawLine(
					pieceX + 0.25 * pieceWidth, pieceY + 0.25 * pieceHeight,
					pieceX + 0.75 * pieceWidth, pieceY + 0.75 * pieceHeight
				);
				painter.drawLine(
					pieceX + 0.75 * pieceWidth, pieceY + 0.25 * pieceHeight,
					pieceX + 0.25 * pieceWidth, pieceY + 0.75 * pieceHeight
				);
			}
			//draw lines to connected neighbors
			foreach (Palapeli::Piece::NeighborInfo neighbor, piece->m_neighbors)
			{
				if (neighborsAlreadyConnected.contains(neighbor.piece))
					continue; //do not draw connection twice (1 -> 2 and 2 -> 1)
				if (neighbor.piece->part() != piece->part())
					continue;
				const QRectF neighborRect = neighbor.piece->sceneBoundingRect();
				const qreal neighborX = neighborRect.x(), neighborY = neighborRect.y();
				const qreal neighborWidth = neighborRect.width(), neighborHeight = neighborRect.height();
				const qreal neighborCenterX = qBound(0.0, neighborX + 0.5 * neighborWidth, sceneWidth);
				const qreal neighborCenterY = qBound(0.0, neighborY + 0.5 * neighborHeight, sceneHeight);
				painter.drawLine(neighborCenterX, neighborCenterY, pieceCenterX, pieceCenterY);
			}
			neighborsAlreadyConnected << piece;
		}
	}
}

#include "minimap.moc"

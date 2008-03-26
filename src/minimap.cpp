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

#include <QPainter>

Palapeli::Minimap::Minimap(Palapeli::View* view)
	: QWidget()
	, m_view(view)
	, m_scene(view->puzzleScene())
{
	setBackgroundRole(QPalette::Window);
	connect(m_scene, SIGNAL(minimapNeedsUpdate()), this, SLOT(update()));
	connect(m_view, SIGNAL(viewportMoved()), this, SLOT(update()));
	//debug
	setWindowTitle("Minimap - Palapeli");
	resize(250, 250);
}

Palapeli::Minimap::~Minimap()
{
}

void Palapeli::Minimap::paintEvent(QPaintEvent*) //friend of Palapeli::Piece
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	const QSizeF sceneSize = m_scene->sceneRect().size();
	const qreal sceneWidth = sceneSize.width(), sceneHeight = sceneSize.height();
	painter.scale(width() / sceneWidth, height() / sceneHeight);
	const QPen pen(palette().highlight().color());

	//draw view rectangle
	QRect viewRect(0, 0, m_view->width(), m_view->height());
	painter.setBrush(palette().base());
	painter.drawPolygon(m_view->mapToScene(viewRect));
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

#include "minimap.moc"

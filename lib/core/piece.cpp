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

#include "piece.h"
#include "manager.h"
#include "part.h"
#include "view.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>

Palapeli::Piece::Piece(const QPixmap& pixmap, const QRectF& positionInImage)
	: QGraphicsPixmapItem(pixmap)
	, m_part(0)
	, m_positionInImage(positionInImage)
	, m_moving(false)
{
	setAcceptedMouseButtons(Qt::LeftButton);
	setOffset(0.0, 0.0);

	ppMgr()->view()->realScene()->addItem(this);
}

Palapeli::Piece* Palapeli::Piece::fromPixmapPair(const QPixmap& pixmap, const QPixmap& mask, const QRectF& positionInImage)
{
	//return new Palapeli::Piece(pixmap, positionInImage);
	QPixmap compositePixmap(mask); //need a non-const QPixmap
	QPainter painter(&compositePixmap);
	painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
	painter.drawPixmap(QPoint(0, 0), pixmap);
	painter.end();
	return new Palapeli::Piece(compositePixmap, positionInImage);
}

QPointF Palapeli::Piece::positionInImage() const
{
	return m_positionInImage.topLeft();
}

QSizeF Palapeli::Piece::size() const
{
	return m_positionInImage.size();
}

Palapeli::Part* Palapeli::Piece::part() const
{
	return m_part;
}

void Palapeli::Piece::setPart(Palapeli::Part* part)
{
	m_part = part;
}

void Palapeli::Piece::makePositionValid(QPointF& basePosition) const
{
	//add constraints to the given base position to make this piece be placed in the scene boundaries
	const QRectF sceneRect = ppMgr()->view()->realScene()->sceneRect();
	basePosition.rx() = qBound(sceneRect.left() - m_positionInImage.left(), basePosition.x(), sceneRect.right() - m_positionInImage.right());
	basePosition.ry() = qBound(sceneRect.top() - m_positionInImage.top(), basePosition.y(), sceneRect.bottom() - m_positionInImage.bottom());
}

void Palapeli::Piece::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if (m_part)
	{
		m_moving = event->button() & Qt::LeftButton;
		m_grabPosition = event->scenePos() - m_part->basePosition();
	}
}

void Palapeli::Piece::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	if (m_part && m_moving)
	{
		m_part->move(event->scenePos() - m_grabPosition);
	}
}

void Palapeli::Piece::mouseReleaseEvent(QGraphicsSceneMouseEvent*)
{
	if (m_part && m_moving)
	{
		ppMgr()->pieceMoveFinished();
		m_moving = false;
	}
}
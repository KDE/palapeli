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

#include "piece.h"
#include "scene.h"

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QPalette>
#include <QPropertyAnimation>

Palapeli::Piece::Piece(const Palapeli::PieceVisuals& pieceVisuals, const Palapeli::PieceVisuals& shadowVisuals)
	: m_pieceItem(new QGraphicsPixmapItem(pieceVisuals.pixmap, this))
	, m_inactiveShadowItem(0)
	, m_activeShadowItem(0)
	, m_animator(0)
{
	m_pieceItem->setAcceptedMouseButtons(Qt::LeftButton);
	m_pieceItem->setOffset(pieceVisuals.offset);
	if (!shadowVisuals.isNull())
		createShadowItems(shadowVisuals);
	//initialize behavior of this item
	setCursor(Qt::OpenHandCursor);
	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemIsSelectable);
	setHandlesChildEvents(true);
}

//BEGIN visuals

void Palapeli::Piece::createShadow()
{
	if (!m_inactiveShadowItem)
		createShadowItems(Palapeli::createShadow(pieceVisuals(), m_atomicSize));
}

void Palapeli::Piece::createShadowItems(const Palapeli::PieceVisuals& shadowVisuals)
{
	const QColor activeShadowColor = QApplication::palette().color(QPalette::Highlight);
	const Palapeli::PieceVisuals activeShadowVisuals = Palapeli::changeShadowColor(shadowVisuals, activeShadowColor);
	//create inactive shadow item
	m_inactiveShadowItem = new QGraphicsPixmapItem(shadowVisuals.pixmap, this);
	m_inactiveShadowItem->setOffset(shadowVisuals.offset);
	m_inactiveShadowItem->setZValue(-2);
	m_inactiveShadowItem->setCursor(cursor());
	//create active shadow item and animator for its opacity
	m_activeShadowItem = new QGraphicsPixmapItem(activeShadowVisuals.pixmap, this);
	m_activeShadowItem->setOffset(activeShadowVisuals.offset);
	m_activeShadowItem->setZValue(-1);
	m_activeShadowItem->setCursor(cursor());
	m_activeShadowItem->setOpacity(isSelected() ? 1.0 : 0.0);
	m_animator = new QPropertyAnimation(this, "activeShadowOpacity", this);
}

QRectF Palapeli::Piece::boundingRect() const
{
	//for proper activation
	return childrenBoundingRect();
}

QRectF Palapeli::Piece::bareBoundingRect() const
{
	return m_pieceItem->boundingRect();
}

QRectF Palapeli::Piece::sceneBareBoundingRect() const
{
	return mapToScene(bareBoundingRect()).boundingRect();
}

Palapeli::PieceVisuals Palapeli::Piece::pieceVisuals() const
{
	Palapeli::PieceVisuals result = { m_pieceItem->pixmap(), m_pieceItem->offset().toPoint() };
	return result;
}

Palapeli::PieceVisuals Palapeli::Piece::shadowVisuals() const
{
	if (!m_inactiveShadowItem)
		return Palapeli::PieceVisuals();
	Palapeli::PieceVisuals result = { m_inactiveShadowItem->pixmap(), m_inactiveShadowItem->offset().toPoint() };
	return result;
}

qreal Palapeli::Piece::activeShadowOpacity() const
{
	return m_activeShadowItem ? m_activeShadowItem->opacity() : 0.0;
}

void Palapeli::Piece::setActiveShadowOpacity(qreal opacity)
{
	if (m_activeShadowItem)
		m_activeShadowItem->setOpacity(opacity);
}

QVariant Palapeli::Piece::itemChange(GraphicsItemChange change, const QVariant& value)
{
	if (change == ItemSelectedChange)
	{
		const qreal targetOpacity = value.toBool() ? 1.0 : 0.0;
		const qreal opacityDiff = qAbs(targetOpacity - activeShadowOpacity());
		if (opacityDiff != 0)
		{
			m_animator->setDuration(150 * opacityDiff);
			m_animator->setStartValue(activeShadowOpacity());
			m_animator->setEndValue(targetOpacity);
			m_animator->start();
		}
	}
	return QGraphicsObject::itemChange(change, value);
}

//END visuals
//BEGIN internal datastructures

void Palapeli::Piece::addRepresentedAtomicPieces(const QList<int>& representedAtomicPieces)
{
	foreach (int id, representedAtomicPieces)
		if (!m_representedAtomicPieces.contains(id))
			m_representedAtomicPieces << id;
}

QList<int> Palapeli::Piece::representedAtomicPieces() const
{
	return m_representedAtomicPieces;
}

void Palapeli::Piece::addLogicalNeighbors(const QList<Palapeli::Piece*>& logicalNeighbors)
{
	foreach (Palapeli::Piece* piece, logicalNeighbors)
		if (!m_logicalNeighbors.contains(piece))
			m_logicalNeighbors << piece;
}

QList<Palapeli::Piece*> Palapeli::Piece::logicalNeighbors() const
{
	return m_logicalNeighbors;
}

void Palapeli::Piece::rewriteLogicalNeighbors(const QList<Palapeli::Piece*>& oldPieces, Palapeli::Piece* newPiece)
{
	bool oldPiecesFound = false;
	foreach (Palapeli::Piece* oldPiece, oldPieces)
	{
		int index = m_logicalNeighbors.indexOf(oldPiece);
		if (index != -1)
		{
			oldPiecesFound = true;
			m_logicalNeighbors.removeAt(index);
		}
	}
	if (oldPiecesFound && newPiece) //newPiece == 0 allows to just drop the old pieces
		m_logicalNeighbors += newPiece;
}

void Palapeli::Piece::addAtomicSize(const QSize& size)
{
	m_atomicSize = m_atomicSize.expandedTo(size);
}

QSize Palapeli::Piece::atomicSize() const
{
	return m_atomicSize;
}

//END internal datastructures
//BEGIN mouse interaction

void Palapeli::Piece::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	QGraphicsItem::mousePressEvent(event); //handle dragging
	if (event->button() == Qt::LeftButton)
	{
		//move item to top
		static int zValue = 0;
		setZValue(++zValue);
		//indicate dragging
		setCursor(Qt::ClosedHandCursor);
		//hack because cursors do not properly propagate to child items
		m_pieceItem->setCursor(Qt::ClosedHandCursor);
		m_inactiveShadowItem->setCursor(Qt::ClosedHandCursor);
		m_activeShadowItem->setCursor(Qt::ClosedHandCursor);
	}
}

void Palapeli::Piece::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	QGraphicsItem::mouseMoveEvent(event); //handle dragging
	if (event->buttons() & Qt::LeftButton)
	{
		Palapeli::Scene* scene = qobject_cast<Palapeli::Scene*>(this->scene());
		if (scene)
		{
			scene->validatePiecePosition(this);
			scene->updateSavegame();
		}
	}
}

void Palapeli::Piece::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	QGraphicsItem::mouseReleaseEvent(event); //handle dragging
	if (event->button() == Qt::LeftButton)
	{
		//indicate end of dragging
		setCursor(Qt::OpenHandCursor);
		//hack because cursors do not properly propagate to child items
		m_pieceItem->setCursor(Qt::OpenHandCursor);
		m_inactiveShadowItem->setCursor(Qt::OpenHandCursor);
		m_activeShadowItem->setCursor(Qt::OpenHandCursor);
		//propagate end of move
		emit moved();
	}
}

//END mouse interaction

#include "piece.moc"

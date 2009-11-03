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

#include "part.h"
#include "piece.h"
#include "scene.h"
#include "settings.h"
#include "shadowitem.h"

#include <QCursor>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPropertyAnimation>
#include <QSet>

Palapeli::Part::Part(Palapeli::Piece* piece)
{
	m_pieces << piece;
	piece->setParentItem(this);
	piece->setCursor(Qt::OpenHandCursor);
	setFlag(QGraphicsItem::ItemIsMovable);
	setHandlesChildEvents(true);
	//add shadow to piece
	const QPixmap piecePixmap = piece->pixmapItem()->pixmap();
	const QSize pixmapSize = piecePixmap.size();
	const int radius = 0.15 * (piecePixmap.width() + piecePixmap.height());
	Palapeli::ShadowItem* shadowItem = new Palapeli::ShadowItem(piecePixmap, radius, piece->pixmapItem()->offset());
	m_shadows << shadowItem;
	shadowItem->setParentItem(this);
	shadowItem->setZValue(-10);
}

Palapeli::Part::~Part()
{
}

void Palapeli::Part::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	QGraphicsItem::mousePressEvent(event); //handle dragging
	if (event->button() == Qt::LeftButton)
	{
		//move item to top
		static int zValue = 0;
		setZValue(++zValue);
		foreach (Palapeli::Piece* piece, m_pieces)
			piece->setCursor(Qt::ClosedHandCursor);
	}
}

void Palapeli::Part::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	QGraphicsItem::mouseMoveEvent(event); //handle dragging
	if (event->buttons() & Qt::LeftButton)
	{
		validatePosition();
		emit partMoving();
	}
}

void Palapeli::Part::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	QGraphicsItem::mouseReleaseEvent(event); //handle dragging
	if (event->button() == Qt::LeftButton)
	{
		searchConnections();
		emit partMoved();
		foreach (Palapeli::Piece* piece, m_pieces)
			piece->setCursor(Qt::OpenHandCursor);
	}
}

bool Palapeli::Part::searchConnections()
{
	//check for parts that can be merged with this part
	QSet<Palapeli::Part*> mergeParts;
	qreal snappingPrecision = qreal(Settings::snappingPrecision()) / 100.0;
	foreach (Palapeli::Piece* piece, m_pieces)
	{
		const QList<Palapeli::Piece*> mergeNeighbors = piece->connectableNeighbors(snappingPrecision);
		foreach (Palapeli::Piece* mergeNeighbor, mergeNeighbors)
			mergeParts << mergeNeighbor->part();
	}
	if (mergeParts.isEmpty())
		return false;
	//merge any parts that are near this part
	foreach (Palapeli::Part* part, mergeParts)
	{
		//set position in such a way that the majority of the pieces do not move
		QPointF posDiff = part->pos() - pos();
		const bool useAnimations = part->pos() != pos();
		QList<QObject*> animatedObjects;
		const bool reversePositioningOrder = part->m_pieces.count() > m_pieces.count();
		if (reversePositioningOrder)
		{
			setPos(part->pos());
			//animate all old pieces to move to the new position
			posDiff = -posDiff;
			if (useAnimations)
			{
				foreach (Palapeli::Piece* piece, m_pieces)
					animatedObjects << piece;
				foreach (Palapeli::ShadowItem* shadowItem, m_shadows)
					animatedObjects << shadowItem;
			}
		}
		//insert all pieces of the other part into this part
		foreach (Palapeli::Piece* piece, part->m_pieces)
		{
			//move piece to this parent
			piece->setParentItem(this);
			m_pieces << piece;
			if (useAnimations && !reversePositioningOrder)
				animatedObjects << piece;
		}
		foreach (Palapeli::ShadowItem* shadowItem, part->m_shadows)
		{
			//move shadow item to this parent
			shadowItem->setParentItem(this);
			m_shadows << shadowItem;
			if (useAnimations && !reversePositioningOrder)
				animatedObjects << shadowItem;
		}
		//start animations
		if (useAnimations)
			foreach (QObject* object, animatedObjects)
			{
				QPropertyAnimation* anim = new QPropertyAnimation(object, "pos", object);
				anim->setStartValue(posDiff);
				anim->setEndValue(QPointF());
				anim->setDuration(200);
				anim->setEasingCurve(QEasingCurve::InCubic);
				anim->start(QAbstractAnimation::DeleteWhenStopped);
			}
		delete part;
	}
	//update internal neighbor lists in the pieces
	foreach (Palapeli::Piece* piece, m_pieces)
		piece->updateNeighborsList();
	//make position valid again
	validatePosition();
	return true;
}

QRectF Palapeli::Part::piecesBoundingRect() const
{
	QRectF boundingRect;
	foreach (Palapeli::Piece* piece, m_pieces)
		boundingRect |= piece->mapToParent(piece->childrenBoundingRect()).boundingRect();
	return boundingRect;
}

QRectF Palapeli::Part::scenePiecesBoundingRect() const
{
	return sceneTransform().mapRect(piecesBoundingRect());
}

void Palapeli::Part::validatePosition()
{
	Palapeli::Scene* s = qobject_cast<Palapeli::Scene*>(scene());
	if (!s)
		return;
	if (s->isConstrained())
	{
		//scene rect constraint is active -> ensure that part stays inside scene rect
		const QRectF sr = s->sceneRect();
		const QRectF br = sceneTransform().mapRect(piecesBoundingRect()); //br = bounding rect
		if (!sr.contains(br))
		{
			QPointF pos = this->pos();
			if (br.left() < sr.left())
				pos.rx() += sr.left() - br.left();
			if (br.right() > sr.right())
				pos.rx() += sr.right() - br.right();
			if (br.top() < sr.top())
				pos.ry() += sr.top() - br.top();
			if (br.bottom() > sr.bottom())
				pos.ry() += sr.bottom() - br.bottom();
			setPos(pos);
		}
	}
}

#include "part.moc"

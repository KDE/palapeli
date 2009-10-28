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

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#if QT_VERSION >= 0x040600
#	include <QPropertyAnimation>
#endif
#include <QSet>

Palapeli::Part::Part(Palapeli::Piece* piece)
{
	m_pieces << piece;
	piece->setParentItem(this);
	setFlag(QGraphicsItem::ItemIsMovable);
	setHandlesChildEvents(true);
	//add shadow to piece
	const QSize pixmapSize = piece->pixmap().size();
	const int radius = 0.15 * (pixmapSize.width() + pixmapSize.height());
	Palapeli::ShadowItem* shadowItem = new Palapeli::ShadowItem(piece->pixmap(), radius, piece->offset());
	m_shadows << shadowItem;
	shadowItem->setParentItem(this);
	shadowItem->setZValue(-10);
}

Palapeli::Part::~Part()
{
}

void Palapeli::Part::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	QGraphicsItem::mouseReleaseEvent(event); //handle dragging
	if (event->button() == Qt::LeftButton)
	{
		//move item to top
		static int zValue = 1;
		setZValue(zValue);
		++zValue;
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
		const QPointF posDiff = part->pos() - pos();
		const bool useAnimations = part->pos() != pos();
		const bool reversePositioningOrder = part->m_pieces.count() > m_pieces.count();
		if (reversePositioningOrder)
		{
			setPos(part->pos());
			//instead of animating the new pieces, animate all old pieces to move to the new position
#if QT_VERSION >= 0x040600
			if (useAnimations)
			{
				QList<QObject*> objects;
				foreach (Palapeli::Piece* piece, m_pieces)
					objects << piece;
				foreach (Palapeli::ShadowItem* shadowItem, m_shadows)
					objects << shadowItem;
				foreach (QObject* object, objects)
				{
					QPropertyAnimation* anim = new QPropertyAnimation(object, "pos", object);
					anim->setStartValue(-posDiff);
					anim->setEndValue(QPointF());
					anim->setDuration(200);
					anim->setEasingCurve(QEasingCurve::InCubic);
					anim->start(QAbstractAnimation::DeleteWhenStopped);
				}
			}
#endif
		}
		//insert all pieces of the other part into this part
		foreach (Palapeli::Piece* piece, part->m_pieces)
		{
			//move piece to this parent
			piece->setParentItem(this);
			m_pieces << piece;
			//animate move to new position inside this parent
#if QT_VERSION >= 0x040600
			if (useAnimations && !reversePositioningOrder)
			{
				QPropertyAnimation* anim = new QPropertyAnimation(piece, "pos", piece);
				anim->setStartValue(posDiff);
				anim->setEndValue(QPointF());
				anim->setDuration(200);
				anim->setEasingCurve(QEasingCurve::InCubic);
				anim->start(QAbstractAnimation::DeleteWhenStopped);
			}
#endif
		}
		foreach (Palapeli::ShadowItem* shadowItem, part->m_shadows)
		{
			shadowItem->setParentItem(this);
			m_shadows << shadowItem;
			//animate move to new position inside this parent
#if QT_VERSION >= 0x040600
			if (useAnimations && !reversePositioningOrder)
			{
				QPropertyAnimation* anim = new QPropertyAnimation(shadowItem, "pos", shadowItem);
				anim->setStartValue(posDiff);
				anim->setEndValue(QPointF());
				anim->setDuration(200);
				anim->setEasingCurve(QEasingCurve::InCubic);
				anim->start(QAbstractAnimation::DeleteWhenStopped);
			}
#endif
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
	{
		const QRectF subRect = piece->mapToParent(piece->boundingRect()).boundingRect();
		if (!boundingRect.isValid())
			boundingRect = subRect;
		else
			boundingRect = boundingRect.united(subRect);
	}
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

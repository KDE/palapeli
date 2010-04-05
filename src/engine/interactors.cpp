/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>
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

#include "interactors.h"
#include "piece.h"
#include "view.h"

#include <QScrollBar>
#include <QStyle>
#include <QStyleOptionRubberBand>

//BEGIN Palapeli::MovePieceInteractor

Palapeli::MovePieceInteractor::MovePieceInteractor(QGraphicsView* view)
	: Palapeli::Interactor(Palapeli::MouseInteractor, view)
{
}

bool Palapeli::MovePieceInteractor::acceptItemUnderMouse(QGraphicsItem* item)
{
	//we are currently moving something -> grab all events
	if (!m_currentPieces.isEmpty())
		return true;
	//we are not moving anything -> is there anything at all under the mouse?
	if (!item)
		return false;
	//we will only move pieces -> find the piece which we are moving
	Palapeli::Piece* piece = findPieceForItem(item);
	if (!piece)
		return false;
	//start moving this piece
	determineSelectedItems(item, piece);
	return true;
}

Palapeli::Piece* Palapeli::MovePieceInteractor::findPieceForItem(QGraphicsItem* mouseInteractingItem) const
{
	//try to cast the item and its parent
	Palapeli::Piece* piece = qgraphicsitem_cast<Palapeli::Piece*>(mouseInteractingItem);
	return piece ? piece : qgraphicsitem_cast<Palapeli::Piece*>(mouseInteractingItem->parentItem());
}

void Palapeli::MovePieceInteractor::determineSelectedItems(QGraphicsItem* clickedItem, Palapeli::Piece* clickedPiece)
{
	m_currentItems.clear();
	m_currentPieces.clear();
	const QList<QGraphicsItem*> selectedItems = clickedItem->scene()->selectedItems();
	if (clickedItem->isSelected())
	{
		//clicked item is already selected -> include all selected items/pieces in this move
		foreach (QGraphicsItem* selectedItem, selectedItems)
		{
			Palapeli::Piece* selectedPiece = findPieceForItem(selectedItem);
			if (selectedPiece)
			{
				m_currentItems << selectedItem;
				m_currentPieces << selectedPiece;
			}
		}
		//NOTE: clickedItem is in the list selectedItems, so it need not be handled separately.
	}
	else
	{
		//clicked item is not selected -> deselect everything else and select only this item
		foreach (QGraphicsItem* selectedItem, selectedItems)
			selectedItem->setSelected(false);
		clickedItem->setSelected(true);
		m_currentItems << clickedItem;
		m_currentPieces << clickedPiece;
	}
}

void Palapeli::MovePieceInteractor::mousePressEvent(const Palapeli::InteractorMouseEvent& event)
{
	//begin moving
	m_baseScenePosition = event.scenePos;
	m_basePositions.clear();
	foreach(Palapeli::Piece* piece, m_currentPieces)
	{
		m_basePositions << piece->pos();
		piece->beginMove();
	}
}

void Palapeli::MovePieceInteractor::mouseMoveEvent(const Palapeli::InteractorMouseEvent& event)
{
	for (int i = 0; i < m_currentPieces.count(); ++i)
	{
		m_currentPieces[i]->setPos(m_basePositions[i] + event.scenePos - m_baseScenePosition);
		m_currentPieces[i]->doMove();
	}
}

void Palapeli::MovePieceInteractor::mouseReleaseEvent(const Palapeli::InteractorMouseEvent& event)
{
	Q_UNUSED(event)
	foreach(Palapeli::Piece* piece, m_currentPieces)
		piece->endMove();
	m_currentItems.clear();
	m_currentPieces.clear();
}

//END Palapeli::MovePieceInteractor
//BEGIN Palapeli::MoveViewportInteractor

Palapeli::MoveViewportInteractor::MoveViewportInteractor(QGraphicsView* view)
	: Palapeli::Interactor(Palapeli::MouseInteractor, view)
{
}

void Palapeli::MoveViewportInteractor::mouseMoveEvent(const Palapeli::InteractorMouseEvent& event)
{
	QGraphicsView* v = view();
	const QPointF delta = event.pos - event.lastPos;
	v->horizontalScrollBar()->setValue(v->horizontalScrollBar()->value() + (v->isRightToLeft() ? delta.x() : -delta.x()));
	v->verticalScrollBar()->setValue(v->verticalScrollBar()->value() - delta.y());
}

//END Palapeli::MoveViewportInteractor
//BEGIN Palapeli::ZoomViewportInteractor

Palapeli::ZoomViewportInteractor::ZoomViewportInteractor(QGraphicsView* view)
	: Palapeli::Interactor(Palapeli::WheelInteractor, view)
{
}

void Palapeli::ZoomViewportInteractor::wheelEvent(const Palapeli::InteractorWheelEvent& event)
{
	Palapeli::View* view = qobject_cast<Palapeli::View*>(this->view());
	if (view)
		view->zoomBy(event.delta);
}

//END Palapeli::ZoomViewportInteractor
//BEGIN Palapeli::RubberBandItem

Palapeli::RubberBandItem::RubberBandItem(QGraphicsItem* parent)
	: QGraphicsItem(parent)
{
}

QRectF Palapeli::RubberBandItem::rect() const
{
	return m_rect;
}

void Palapeli::RubberBandItem::setRect(const QRectF& rect)
{
	if (m_rect == rect || (m_rect.isEmpty() && rect.isEmpty()))
		return;
	prepareGeometryChange();
	m_rect = rect;
	update();
	//update list of selected items when rubberband is visible
	if (!rect.isEmpty())
	{
		QPainterPath p;
		p.addRect(sceneBoundingRect());
		scene()->setSelectionArea(p, Qt::ContainsItemBoundingRect);
	}
}

QRectF Palapeli::RubberBandItem::boundingRect() const
{
	return m_rect; //The QStyle does not paint outside this rect!
}

void Palapeli::RubberBandItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(painter)
	Q_UNUSED(option)
	if (m_rect.isEmpty())
		return;
	//find the view which we are painting on (we could be painting directly, or on a viewport contained in the view)
	QGraphicsView* view = qobject_cast<QGraphicsView*>(widget);
	if (!view)
		view = qobject_cast<QGraphicsView*>(widget->parent());
	Q_ASSERT(view);
	//render on view directly
	QPainter viewPainter(widget);
	QRect rect = view->mapFromScene(sceneBoundingRect()).boundingRect();
	QStyleOptionRubberBand opt;
	opt.initFrom(widget);
	opt.rect = rect;
	opt.shape = QRubberBand::Rectangle;
	//painter clipping for masked rubberbands
	QStyleHintReturnMask mask;
	if (widget->style()->styleHint(QStyle::SH_RubberBand_Mask, &opt, widget, &mask))
		painter->setClipRegion(mask.region, Qt::IntersectClip);
	//draw rubberband
	widget->style()->drawControl(QStyle::CE_RubberBand, &opt, &viewPainter, widget);
}

//END Palapeli::RubberBandItem
//BEGIN Palapeli::RubberBandInteractor

Palapeli::RubberBandInteractor::RubberBandInteractor(QGraphicsView* view)
	: Palapeli::Interactor(Palapeli::MouseInteractor, view)
	, m_item(new Palapeli::RubberBandItem)
{
	if (scene())
		scene()->addItem(m_item);
	m_item->hide(); //NOTE: This is not necessary for the painting, but we use m_item->isVisible() to determine whether we are rubberbanding at the moment.
}

Palapeli::RubberBandInteractor::~RubberBandInteractor()
{
	delete m_item;
}

void Palapeli::RubberBandInteractor::sceneChangeEvent(QGraphicsScene* oldScene, QGraphicsScene* newScene)
{
	const bool isVisible = m_item->isVisible();
	if (oldScene)
		oldScene->removeItem(m_item);
	if (newScene)
		newScene->addItem(m_item);
	m_item->setVisible(isVisible); //just to be sure that the scene change does not break the visibility setting
}

bool Palapeli::RubberBandInteractor::acceptItemUnderMouse(QGraphicsItem* item)
{
	if (item == m_item || m_item->isVisible())
		return true; //the rubber band item should not recieve any events
	else
		return item == 0; //while not active, do not accept events when an item is under the mouse
}

void Palapeli::RubberBandInteractor::mousePressEvent(const Palapeli::InteractorMouseEvent& event)
{
	m_basePosition = event.scenePos;
	m_item->show(); //NOTE: This is not necessary for the painting, but we use m_item->isVisible() to determine whether we are rubberbanding at the moment.
	m_item->setRect(QRectF(m_basePosition, QSizeF()));
	m_item->scene()->setSelectionArea(QPainterPath()); //deselect everything
}

void Palapeli::RubberBandInteractor::mouseMoveEvent(const Palapeli::InteractorMouseEvent& event)
{
	//let the interactor pick up the mouse move event only if rubberbanding is actually active (TODO: this is a bug in InteractorManager -> it should recognize chains of mouse events consisting of a series of press-move-move-...-move-release events)
	if (!m_item->isVisible())
		return;
	QSizeF size(event.scenePos.x() - m_basePosition.x(), event.scenePos.y() - m_basePosition.y());
	QRectF rect(m_basePosition, size);
	m_item->setRect(rect.normalized());
}

void Palapeli::RubberBandInteractor::mouseReleaseEvent(const Palapeli::InteractorMouseEvent& event)
{
	if (!m_item->isVisible())
		return;
	Q_UNUSED(event)
	m_item->hide();
	m_item->setRect(QRectF());
}

//END Palapeli::RubberBandInteractor

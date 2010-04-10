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
#include <KLocalizedString>

//BEGIN Palapeli::MovePieceInteractor

Palapeli::MovePieceInteractor::MovePieceInteractor(QGraphicsView* view)
	: Palapeli::Interactor(Palapeli::MouseInteractor, view)
{
	setMetadata(PieceInteraction, i18nc("Description (used like a name) for a mouse interaction method", "Move pieces by dragging"), QIcon());
}

static QGraphicsItem* findSelectableItemAt(const QPointF& scenePos, QGraphicsScene* scene)
{
	QList<QGraphicsItem*> itemsUnderMouse = scene->items(scenePos);
	foreach (QGraphicsItem* itemUnderMouse, itemsUnderMouse)
		if (itemUnderMouse->flags() && QGraphicsItem::ItemIsSelectable)
			return itemUnderMouse;
	return 0;
}

bool Palapeli::MovePieceInteractor::acceptMousePosition(const QPoint& pos)
{
	if (!scene())
		return false;
	//we are currently moving something -> grab all events
	if (!m_currentPieces.isEmpty())
		return true;
	//no item under mouse -> no interaction possible
	QGraphicsItem* selectableItemUnderMouse = findSelectableItemAt(view()->mapToScene(pos), scene());
	if (!selectableItemUnderMouse)
		return false;
	//we will only move pieces -> find the piece which we are moving
	Palapeli::Piece* piece = Palapeli::Piece::fromSelectedItem(selectableItemUnderMouse);
	if (!piece)
		return false;
	//start moving this piece
	determineSelectedItems(selectableItemUnderMouse, piece);
	return true;
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
			Palapeli::Piece* selectedPiece = Palapeli::Piece::fromSelectedItem(selectedItem);
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

void Palapeli::MovePieceInteractor::mousePressEvent(const Palapeli::MouseEvent& event)
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

void Palapeli::MovePieceInteractor::mouseMoveEvent(const Palapeli::MouseEvent& event)
{
	for (int i = 0; i < m_currentPieces.count(); ++i)
	{
		m_currentPieces[i]->setPos(m_basePositions[i] + event.scenePos - m_baseScenePosition);
		m_currentPieces[i]->doMove();
	}
}

void Palapeli::MovePieceInteractor::mouseReleaseEvent(const Palapeli::MouseEvent& event)
{
	Q_UNUSED(event)
	foreach(Palapeli::Piece* piece, m_currentPieces)
		piece->endMove();
	m_currentItems.clear();
	m_currentPieces.clear();
}

//END Palapeli::MovePieceInteractor
//BEGIN Palapeli::SelectPieceInteractor

Palapeli::SelectPieceInteractor::SelectPieceInteractor(QGraphicsView* view)
	: Palapeli::Interactor(Palapeli::MouseInteractor, view)
{
	setMetadata(PieceInteraction, i18nc("Description (used like a name) for a mouse interaction method", "Select pieces by clicking"), QIcon());
}

bool Palapeli::SelectPieceInteractor::acceptMousePosition(const QPoint& pos)
{
	if (!scene())
		return false;
	//no item under mouse -> no interaction possible
	QGraphicsItem* selectableItemUnderMouse = findSelectableItemAt(view()->mapToScene(pos), scene());
	if (!selectableItemUnderMouse)
		return false;
	//we will only move pieces -> find the piece which we are moving
	m_currentPiece = Palapeli::Piece::fromSelectedItem(selectableItemUnderMouse);
	return (bool) m_currentPiece;
}

void Palapeli::SelectPieceInteractor::mousePressEvent(const Palapeli::MouseEvent& event)
{
	Q_UNUSED(event)
	//toggle selection state for piece under mouse
	m_currentPiece->setSelected(!m_currentPiece->isSelected());
}

//END Palapeli::SelectPieceInteractor
//BEGIN Palapeli::MoveViewportInteractor

Palapeli::MoveViewportInteractor::MoveViewportInteractor(QGraphicsView* view)
	: Palapeli::Interactor(Palapeli::MouseInteractor, view)
{
	setMetadata(ViewportInteraction, i18nc("Description (used like a name) for a mouse interaction method", "Move viewport by dragging"), QIcon());
}

void Palapeli::MoveViewportInteractor::mousePressEvent(const Palapeli::MouseEvent& event)
{
	m_lastPos = event.pos;
}

void Palapeli::MoveViewportInteractor::mouseMoveEvent(const Palapeli::MouseEvent& event)
{
	QGraphicsView* v = view();
	const QPointF delta = event.pos - m_lastPos;
	m_lastPos = event.pos;
	v->horizontalScrollBar()->setValue(v->horizontalScrollBar()->value() + (v->isRightToLeft() ? delta.x() : -delta.x()));
	v->verticalScrollBar()->setValue(v->verticalScrollBar()->value() - delta.y());
}

//END Palapeli::MoveViewportInteractor
//BEGIN Palapeli::ZoomViewportInteractor

Palapeli::ZoomViewportInteractor::ZoomViewportInteractor(QGraphicsView* view)
	: Palapeli::Interactor(Palapeli::WheelInteractor, view)
{
	setMetadata(ViewportInteraction, i18nc("Description (used like a name) for a mouse interaction method", "Zoom viewport"), QIcon());
}

void Palapeli::ZoomViewportInteractor::wheelEvent(const Palapeli::WheelEvent& event)
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
	setMetadata(PieceInteraction, i18nc("Description (used like a name) for a mouse interaction method", "Select multiple pieces at once"), QIcon());
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

bool Palapeli::RubberBandInteractor::acceptMousePosition(const QPoint& pos)
{
	if (!scene())
		return false;
	//are we rubberbanding ATM?
	if (m_item->isVisible())
		return true;
	//check items under mouse
	const QPointF scenePos = view()->mapToScene(pos);
	QList<QGraphicsItem*> itemsUnderMouse = scene()->items(scenePos);
	foreach (QGraphicsItem* itemUnderMouse, itemsUnderMouse)
	{
		if (itemUnderMouse->flags() && QGraphicsItem::ItemIsSelectable)
			return false;
	}
	//no selectable item under mouse
	return true;
}

void Palapeli::RubberBandInteractor::mousePressEvent(const Palapeli::MouseEvent& event)
{
	m_basePosition = event.scenePos;
	m_item->show(); //NOTE: This is not necessary for the painting, but we use m_item->isVisible() to determine whether we are rubberbanding at the moment.
	m_item->setRect(QRectF(m_basePosition, QSizeF()));
	m_item->scene()->setSelectionArea(QPainterPath()); //deselect everything
}

void Palapeli::RubberBandInteractor::mouseMoveEvent(const Palapeli::MouseEvent& event)
{
	//let the interactor pick up the mouse move event only if rubberbanding is actually active
	if (!m_item->isVisible())
		return;
	QSizeF size(event.scenePos.x() - m_basePosition.x(), event.scenePos.y() - m_basePosition.y());
	QRectF rect(m_basePosition, size);
	m_item->setRect(rect.normalized());
}

void Palapeli::RubberBandInteractor::mouseReleaseEvent(const Palapeli::MouseEvent& event)
{
	if (!m_item->isVisible())
		return;
	Q_UNUSED(event)
	m_item->hide();
	m_item->setRect(QRectF());
}

//END Palapeli::RubberBandInteractor

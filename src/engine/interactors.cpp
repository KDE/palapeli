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
#include "scene.h"
#include "view.h"

#include <QStyle>
#include <QStyleOptionRubberBand>
#include <KIcon>
#include <KLocalizedString>

//BEGIN Palapeli::MovePieceInteractor

Palapeli::MovePieceInteractor::MovePieceInteractor(QGraphicsView* view)
	: Palapeli::Interactor(20, Palapeli::MouseInteractor, view) //priority: very high because this is the most important interaction
{
	setMetadata(PieceInteraction, i18nc("Description (used like a name) for a mouse interaction method", "Move pieces by dragging"), KIcon("transform-move"));
}

static QGraphicsItem* findSelectableItemAt(const QPointF& scenePos, QGraphicsScene* scene)
{
	if (!scene)
		return 0;
	QList<QGraphicsItem*> itemsUnderMouse = scene->items(scenePos);
	foreach (QGraphicsItem* itemUnderMouse, itemsUnderMouse)
		if (itemUnderMouse->flags() & QGraphicsItem::ItemIsSelectable)
			return itemUnderMouse;
	return 0;
}

void Palapeli::MovePieceInteractor::determineSelectedItems(QGraphicsItem* clickedItem, Palapeli::Piece* clickedPiece)
{
	m_currentPieces.clear();
	const QList<QGraphicsItem*> selectedItems = clickedItem->scene()->selectedItems();
	if (clickedItem->isSelected())
	{
		//clicked item is already selected -> include all selected items/pieces in this move
		foreach (QGraphicsItem* selectedItem, selectedItems)
		{
			Palapeli::Piece* selectedPiece = Palapeli::Piece::fromSelectedItem(selectedItem);
			if (selectedPiece)
				m_currentPieces << selectedPiece;
		}
		//NOTE: clickedItem is in the list selectedItems, so it need not be handled separately.
	}
	else
	{
		//clicked item is not selected -> deselect everything else and select only this item
		foreach (QGraphicsItem* selectedItem, selectedItems)
			selectedItem->setSelected(false);
		clickedItem->setSelected(true);
		m_currentPieces << clickedPiece;
	}
}

bool Palapeli::MovePieceInteractor::startInteraction(const Palapeli::MouseEvent& event)
{
	//no item under mouse -> no interaction possible
	QGraphicsItem* selectableItemUnderMouse = findSelectableItemAt(event.scenePos, scene());
	if (!selectableItemUnderMouse)
		return false;
	//we will only move pieces -> find the piece which we are moving
	Palapeli::Piece* piece = Palapeli::Piece::fromSelectedItem(selectableItemUnderMouse);
	if (!piece)
		return false;
	//start moving this piece
	determineSelectedItems(selectableItemUnderMouse, piece);
	m_baseScenePosition = event.scenePos;
	m_currentOffset = QPointF();
	m_basePositions.clear();
	foreach(Palapeli::Piece* piece, m_currentPieces)
	{
		m_basePositions << piece->pos();
		connect(piece, SIGNAL(replacedBy(Palapeli::Piece*)), SLOT(pieceReplacedBy(Palapeli::Piece*)), Qt::DirectConnection);
		piece->beginMove();
	}
	return true;
}

void Palapeli::MovePieceInteractor::continueInteraction(const Palapeli::MouseEvent& event)
{
	m_currentOffset = event.scenePos - m_baseScenePosition;
	for (int i = 0; i < m_currentPieces.count(); ++i)
	{
		m_currentPieces[i]->setPos(m_basePositions[i] + m_currentOffset);
		m_currentPieces[i]->doMove();
	}
}

void Palapeli::MovePieceInteractor::pieceReplacedBy(Palapeli::Piece* replacement)
{
	//This slot is triggered when a MergeGroup replaces one of the m_currentPieces by a new piece.
	//remove old piece from data structures
	int index = m_currentPieces.indexOf(reinterpret_cast<Palapeli::Piece*>(sender()));
	m_currentPieces.removeAt(index);
	m_basePositions.removeAt(index);
	//add new piece (might not always be necessary, if the new piece replaces more than one of the selected pieces)
	if (!m_currentPieces.contains(replacement))
	{
		m_currentPieces << replacement;
		m_basePositions << replacement->pos() - m_currentOffset;
	}
}

void Palapeli::MovePieceInteractor::stopInteraction(const Palapeli::MouseEvent& event)
{
	Q_UNUSED(event)
	foreach(Palapeli::Piece* piece, m_currentPieces)
	{
		disconnect(piece, 0, this, 0);
		piece->endMove();
	}
	m_currentPieces.clear();
}

//END Palapeli::MovePieceInteractor
//BEGIN Palapeli::SelectPieceInteractor

Palapeli::SelectPieceInteractor::SelectPieceInteractor(QGraphicsView* view)
	: Palapeli::Interactor(19, Palapeli::MouseInteractor, view) //priority: a bit less than MovePieceInteractor
{
	setMetadata(PieceInteraction, i18nc("Description (used like a name) for a mouse interaction method", "Select pieces by clicking"), KIcon("edit-select"));
}

bool Palapeli::SelectPieceInteractor::startInteraction(const Palapeli::MouseEvent& event)
{
	//no item under mouse -> no interaction possible
	QGraphicsItem* selectableItemUnderMouse = findSelectableItemAt(event.scenePos, scene());
	if (!selectableItemUnderMouse)
		return false;
	//we will only move pieces -> find the piece which we are moving
	Palapeli::Piece* currentPiece = Palapeli::Piece::fromSelectedItem(selectableItemUnderMouse);
	if (!currentPiece)
		return false;
	//toggle selection state for piece under mouse
	currentPiece->setSelected(!currentPiece->isSelected());
	return true;
}

//END Palapeli::SelectPieceInteractor
//BEGIN Palapeli::MoveViewportInteractor

Palapeli::MoveViewportInteractor::MoveViewportInteractor(QGraphicsView* view)
	: Palapeli::Interactor(1, Palapeli::MouseInteractor, view) //priority: very low because specific interaction points (e.g. pieces, scene boundaries) are much more important
{
	setMetadata(ViewportInteraction, i18nc("Description (used like a name) for a mouse interaction method", "Move viewport by dragging"), KIcon("transform-move"));
}

bool Palapeli::MoveViewportInteractor::startInteraction(const Palapeli::MouseEvent& event)
{
	m_lastPos = event.pos;
	return true;
}

void Palapeli::MoveViewportInteractor::continueInteraction(const Palapeli::MouseEvent& event)
{
	Palapeli::View* view = qobject_cast<Palapeli::View*>(this->view());
	if (view)
		view->moveViewportBy(event.pos - m_lastPos);
	m_lastPos = event.pos;
}

//END Palapeli::MoveViewportInteractor
//BEGIN Palapeli::ZoomViewportInteractor

Palapeli::ZoomViewportInteractor::ZoomViewportInteractor(QGraphicsView* view)
	: Palapeli::Interactor(0, Palapeli::WheelInteractor, view) //priority: unused for wheel interactors ATM
{
	setMetadata(ViewportInteraction, i18nc("Description (used like a name) for a mouse interaction method", "Zoom viewport"), KIcon("zoom-in"));
}

void Palapeli::ZoomViewportInteractor::doInteraction(const Palapeli::WheelEvent& event)
{
	Palapeli::View* view = qobject_cast<Palapeli::View*>(this->view());
	if (view)
		view->zoomBy(event.delta);
}

//END Palapeli::ZoomViewportInteractor
//BEGIN Palapeli::ScrollViewportInteractor

Palapeli::ScrollViewportInteractor::ScrollViewportInteractor(Qt::Orientation orientation, QGraphicsView* view)
	: Palapeli::Interactor(0, Palapeli::WheelInteractor, view) //priority: unused for wheel interactors ATM
	, m_orientation(orientation)
{
	QString description;
	if (orientation == Qt::Horizontal)
		description = i18nc("Description (used like a name) for a mouse interaction method", "Scroll viewport horizontally");
	else
		description = i18nc("Description (used like a name) for a mouse interaction method", "Scroll viewport vertically");
	setMetadata(ViewportInteraction, description, QIcon());
}

void Palapeli::ScrollViewportInteractor::doInteraction(const Palapeli::WheelEvent& event)
{
	const QPoint widgetDelta = (m_orientation == Qt::Horizontal) ? QPoint(event.delta, 0) : QPoint(0, event.delta);
	Palapeli::View* view = qobject_cast<Palapeli::View*>(this->view());
	if (view)
		view->moveViewportBy(view->mapToScene(widgetDelta)- view->mapToScene(QPoint()));
}

//END Palapeli::ScrollViewportInteractor
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
	: Palapeli::Interactor(2, Palapeli::MouseInteractor, view) //priority: a bit more than MoveViewport, but still much less than interactions with specific interaction points (e.g. pieces, scene boundaries)
	, m_item(new Palapeli::RubberBandItem)
{
	setMetadata(PieceInteraction, i18nc("Description (used like a name) for a mouse interaction method", "Select multiple pieces at once"), KIcon("select-rectangular"));
	if (scene())
		scene()->addItem(m_item);
	m_item->hide(); //NOTE: This is not necessary for the painting, but we use m_item->isVisible() to determine whether we are rubberbanding at the moment. //FIXME: really?
}

Palapeli::RubberBandInteractor::~RubberBandInteractor()
{
	delete m_item;
}

void Palapeli::RubberBandInteractor::sceneChangeEvent(QGraphicsScene* oldScene, QGraphicsScene* newScene)
{
	if (oldScene)
		oldScene->removeItem(m_item);
	if (newScene)
		newScene->addItem(m_item);
	m_item->setVisible(false);
}

bool Palapeli::RubberBandInteractor::startInteraction(const Palapeli::MouseEvent& event)
{
	//check items under mouse
	if (findSelectableItemAt(event.scenePos, scene()))
		return false;
	//start rubberbanding
	m_basePosition = event.scenePos;
	m_item->show(); //NOTE: This is not necessary for the painting, but we use m_item->isVisible() to determine whether we are rubberbanding at the moment.
	m_item->setRect(QRectF(m_basePosition, QSizeF()));
	scene()->setSelectionArea(QPainterPath()); //deselect everything
	return true;
}

void Palapeli::RubberBandInteractor::continueInteraction(const Palapeli::MouseEvent& event)
{
	QSizeF size(event.scenePos.x() - m_basePosition.x(), event.scenePos.y() - m_basePosition.y());
	QRectF rect(m_basePosition, size);
	m_item->setRect(rect.normalized());
}

void Palapeli::RubberBandInteractor::stopInteraction(const Palapeli::MouseEvent& event)
{
	Q_UNUSED(event)
	m_item->hide(); //NOTE: This is not necessary for the painting, but we use m_item->isVisible() to determine whether we are rubberbanding at the moment.
	m_item->setRect(QRectF());
}

//END Palapeli::RubberBandInteractor
//BEGIN Palapeli::ToggleConstraintInteractor

Palapeli::ToggleConstraintInteractor::ToggleConstraintInteractor(QGraphicsView* view)
	: Palapeli::Interactor(30, Palapeli::MouseInteractor, view) //priority: higher than anything else (this should not depend on what the cursor is currently pointing)
{
	setMetadata(TableInteraction, i18nc("Description (used like a name) for a mouse interaction method", "Toggle lock state of the puzzle table area"), QIcon());
}

bool Palapeli::ToggleConstraintInteractor::startInteraction(const Palapeli::MouseEvent& event)
{
	Q_UNUSED(event)
	Palapeli::Scene* scene = qobject_cast<Palapeli::Scene*>(this->scene());
	if (scene)
		scene->setConstrained(!scene->isConstrained());
	return (bool) scene;
}

//END Palapeli::ToggleConstraintInteractor

#include "interactors.moc"

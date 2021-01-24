/*
    SPDX-FileCopyrightText: 2009, 2010 Stefan Majewsky <majewsky@gmx.net>
    SPDX-FileCopyrightText: 2010 Johannes Löhnert <loehnert.kde@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "piece.h"
#include "piece_p.h"
#include "scene.h"
#include "settings.h"

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QPalette>
#include <QPropertyAnimation>
#include <QPainter> // IDW test.
#include <QRandomGenerator>

void Palapeli::Piece::commonInit(const Palapeli::PieceVisuals& pieceVisuals)
{
	Palapeli::SelectionAwarePixmapItem* pieceItem = new Palapeli::SelectionAwarePixmapItem(pieceVisuals.pixmap(), this);
	connect(pieceItem, &Palapeli::SelectionAwarePixmapItem::selectedChanged, this, &Piece::pieceItemSelectedChanged);
	m_pieceItem = pieceItem;
	m_pieceItem->setOffset(pieceVisuals.offset());
	//initialize behavior
	m_pieceItem->setAcceptedMouseButtons(Qt::LeftButton);
	m_pieceItem->setCursor(Qt::OpenHandCursor);
	m_pieceItem->setFlag(QGraphicsItem::ItemIsSelectable);
	// replacing m_pieceItems pixmap (in rerenderBevel()) causes weird pixel errors
	// when using fast transformation. SmoothTransformation looks better anyway...
	m_pieceItem->setTransformationMode(Qt::SmoothTransformation);
	m_offset = m_pieceItem->offset().toPoint();
}

Palapeli::Piece::Piece(const QImage& pieceImage, const QPoint& offset)
	: m_pieceItem(nullptr)
	, m_inactiveShadowItem(nullptr)
	, m_activeShadowItem(nullptr)
	, m_highlightItem(nullptr)
	, m_animator(nullptr)
	, m_offset(offset)
{
	//create bevel map if wanted
	if (Settings::pieceBevelsEnabled())
	{
		//NOTE: The bevel map calculation and application is divided because we
		//need different result visuals for different turning angles when piece
		//rotation gets implementation.
		const QSize size = pieceImage.size();
		const int radius = 0.04 * (size.width() + size.height());
		const Palapeli::BevelMap bevelMap = Palapeli::calculateBevelMap(pieceImage, radius);
		commonInit(Palapeli::PieceVisuals(Palapeli::applyBevelMap(pieceImage, bevelMap, /* angle= */ 0), offset));
	}
	else
		commonInit(Palapeli::PieceVisuals(pieceImage, offset));
}

Palapeli::Piece::Piece(const Palapeli::PieceVisuals& pieceVisuals, const Palapeli::PieceVisuals& shadowVisuals, const Palapeli::PieceVisuals& highlightVisuals)
	: m_pieceItem(nullptr)
	, m_inactiveShadowItem(nullptr)
	, m_activeShadowItem(nullptr)
	, m_highlightItem(nullptr)
	, m_animator(nullptr)
	, m_offset(QPoint(0, 0))	// Gets set in Piece::commonInit().
{
	commonInit(pieceVisuals);
	if (!shadowVisuals.isNull())
		createShadowItems(shadowVisuals);
	if (!highlightVisuals.isNull()) {
		m_highlightItem = new QGraphicsPixmapItem
					(highlightVisuals.pixmap(), this);
		m_highlightItem->setOffset(highlightVisuals.offset());
		m_highlightItem->setZValue(-1);
		m_highlightItem->setVisible(isSelected());
	}
}

//BEGIN visuals

bool Palapeli::Piece::completeVisuals()
{
	if (Settings::pieceShadowsEnabled() && !m_inactiveShadowItem)
	{
		createShadowItems(Palapeli::createShadow(pieceVisuals(), m_atomicSize));
		return true;
	}
	return false;
}

bool Palapeli::Piece::hasShadow() const
{
	return (bool) m_inactiveShadowItem;
}

bool Palapeli::Piece::hasHighlight() const
{
	return (bool) m_highlightItem;
}

void Palapeli::Piece::createShadowItems(const Palapeli::PieceVisuals& shadowVisuals)
{
/* IDW TODO - DELETE this.
#ifdef Q_OS_MAC
	// On Apple OS X the QPalette::Highlight color is blue, but is
	// dimmed down, for highlighting black-on-white text presumably.
	const QColor activeShadowColor(Qt::cyan);
	// Note: Q_WS_MAC is deprecated and does not exist in Qt 5.
#else
	const QColor activeShadowColor =
		QApplication::palette().color(QPalette::Highlight);
#endif
*/
	const QColor activeShadowColor = Settings::viewHighlightColor();
	const Palapeli::PieceVisuals activeShadowVisuals =
		Palapeli::changeShadowColor(shadowVisuals, activeShadowColor);
	// Create inactive (unhighlighted) shadow item.
	m_inactiveShadowItem = new QGraphicsPixmapItem(shadowVisuals.pixmap(), this);
	m_inactiveShadowItem->setOffset(shadowVisuals.offset());
	m_inactiveShadowItem->setZValue(-2);
	// Create active shadow item (highlighted) and animator for its opacity.
	m_activeShadowItem = new QGraphicsPixmapItem(activeShadowVisuals.pixmap(), this);
	m_activeShadowItem->setOffset(activeShadowVisuals.offset());
	m_activeShadowItem->setZValue(-1);
	m_activeShadowItem->setOpacity(isSelected() ? 1.0 : 0.0);
	m_animator = new QPropertyAnimation(this, "activeShadowOpacity", this);
}

void Palapeli::Piece::createHighlight(const QSizeF& pieceAreaSize)
{
	QRectF rect = sceneBareBoundingRect();
	// IDW TODO - Make the factor an adjustable setting (1.2-2.0).
	QSizeF area = 1.5 * pieceAreaSize;
	int w = area.width();
	int h = area.height();
	// IDW TODO - Paint pixmap just once (in Scene?) and shallow-copy it.
	QRadialGradient g(QPoint(w/2, h/2), qMin(w/2, h/2));
	g.setColorAt(0, Settings::viewHighlightColor());
	g.setColorAt(1,Qt::transparent);

	QPixmap p(w, h);
	p.fill(Qt::transparent);
	QPainter pa;
	pa.begin(&p);
	pa.setPen(Qt::NoPen);
	pa.setBrush(QBrush(g));
	pa.drawEllipse(0, 0, w, h);
	pa.end();

	m_highlightItem = new QGraphicsPixmapItem(p, this);
	m_highlightItem->setOffset(m_offset.x() - w/2 + rect.width()/2,
				   m_offset.y() - h/2 + rect.height()/2);
	m_highlightItem->setZValue(-1);
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
	return Palapeli::PieceVisuals(m_pieceItem->pixmap(), m_pieceItem->offset().toPoint());
}

Palapeli::PieceVisuals Palapeli::Piece::shadowVisuals() const
{
	if (!m_inactiveShadowItem)
		return Palapeli::PieceVisuals();
	return Palapeli::PieceVisuals(m_inactiveShadowItem->pixmap(), m_inactiveShadowItem->offset().toPoint());
}

Palapeli::PieceVisuals Palapeli::Piece::highlightVisuals() const
{
	if (!m_highlightItem)
		return Palapeli::PieceVisuals();
	return Palapeli::PieceVisuals(m_highlightItem->pixmap(), m_highlightItem->offset().toPoint());
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

void Palapeli::Piece::pieceItemSelectedChanged(bool selected)
{
	if (!m_activeShadowItem) {
		// No shadows: use a highlighter.
		if (!m_highlightItem) {
			createHighlight((qobject_cast<Palapeli::Scene*>
				    (scene()))->pieceAreaSize());
		}
		// IDW TODO - Use an animator to change the visibility?
		m_highlightItem->setVisible(selected);
		return;
	}
	//change visibility of active shadow
	// IDW TODO - On select, hide black shadow and brighten highlight.
	// m_inactiveShadowItem->setVisible(! selected);	// IDW test.
	const qreal targetOpacity = selected ? 1.0 : 0.0;
	const qreal opacityDiff = qAbs(targetOpacity - activeShadowOpacity());
	if (m_animator && opacityDiff != 0)
	{
		m_animator->setDuration(150 * opacityDiff);
		m_animator->setStartValue(activeShadowOpacity());
		m_animator->setEndValue(targetOpacity);
		m_animator->start();
	}
}

//END visuals
//BEGIN internal datastructures

void Palapeli::Piece::addRepresentedAtomicPieces(const QList<int>& representedAtomicPieces)
{
	for (int id : representedAtomicPieces)
		if (!m_representedAtomicPieces.contains(id))
			m_representedAtomicPieces << id;
}

QList<int> Palapeli::Piece::representedAtomicPieces() const
{
	return m_representedAtomicPieces;
}

void Palapeli::Piece::addLogicalNeighbors(const QList<Palapeli::Piece*>& logicalNeighbors)
{
	for (Palapeli::Piece* piece : logicalNeighbors)
		// IDW TODO - if (!m_logicalNeighbors.contains(piece) && piece)
		//            If piece == 0, pieceID was not in m_loadedPieces.
		//            This would be an integrity error in .puzzle file.
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
	for (Palapeli::Piece* oldPiece : oldPieces)
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

void Palapeli::Piece::announceReplaced(Palapeli::Piece* replacement)
{
	Q_EMIT replacedBy(replacement);
	deleteLater();
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

void Palapeli::Piece::setPlace(const QPointF& topLeft, int x, int y,
				const QSizeF& area, bool random)
{
	const QRectF b = sceneBareBoundingRect();
	const QSizeF pieceSize = b.size();
	QPointF areaOffset;
	// QPoint pieceOffset = m_pieceItem->offset().toPoint();
	if (random) {
		int dx = area.width() - pieceSize.width();
		int dy = area.height() - pieceSize.height();
                auto *generator = QRandomGenerator::global();
		areaOffset = QPointF(	// Place the piece randomly in the cell.
                        (dx > 0) ? (generator->bounded(dx)) : 0,	// Avoid division by 0.
                        (dy > 0) ? (generator->bounded(dy)) : 0);
	}
	else {
		areaOffset = QPointF(	// Center the piece in the cell.
			(area.width() - pieceSize.width())/2.0,
			(area.height() - pieceSize.height())/2.0);
	}
	const QPointF gridPos(x * area.width(), y * area.height());
	setPos(topLeft + gridPos + areaOffset - m_offset);	// Move it.
}

//BEGIN mouse interaction

bool Palapeli::Piece::isSelected() const
{
	return m_pieceItem->isSelected();
}

void Palapeli::Piece::setSelected(bool selected)
{
	m_pieceItem->setSelected(selected);
}

void Palapeli::Piece::startClick()
{
	m_pieceItem->setCursor(Qt::ClosedHandCursor);	// Button pressed.
}

void Palapeli::Piece::endClick()
{
	m_pieceItem->setCursor(Qt::OpenHandCursor);	// Button released.
}

Palapeli::Piece* Palapeli::Piece::fromSelectedItem(QGraphicsItem* item)
{
	//We expect: item == piece->m_pieceItem && item->parentItem() == piece
	return qgraphicsitem_cast<Palapeli::Piece*>(item->parentItem());
}

void Palapeli::Piece::beginMove()
{
	static int zValue = 0;
	setZValue(++zValue); //move piece to top
	m_pieceItem->setCursor(Qt::ClosedHandCursor);
}

void Palapeli::Piece::doMove()
{
	Palapeli::Scene* scene = qobject_cast<Palapeli::Scene*>(this->scene());
	if (scene) {
		scene->validatePiecePosition(this);
		Q_EMIT moved(false);	// Still moving.
	}
}

void Palapeli::Piece::endMove()
{
	m_pieceItem->setCursor(Qt::OpenHandCursor);
	Q_EMIT moved(true);		// Finishd moving.
}

//END mouse interaction


//

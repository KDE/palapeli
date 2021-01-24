/*
    SPDX-FileCopyrightText: 2009, 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "scene.h"
#include "constraintvisualizer.h"
#include "mergegroup.h"
#include "piece.h"
#include "settings.h"

#include <QGraphicsView>
#include "palapeli_debug.h"

Palapeli::Scene::Scene(QObject* parent)
	: QGraphicsScene(parent)
	, m_constrained(false)
	, m_constraintVisualizer(new Palapeli::ConstraintVisualizer(this))
	, m_puzzle(nullptr)
	, m_pieceAreaSize(QSizeF(32.0, 32.0))	// Allow 1024 pixels initially.
	, m_margin(10.0)
	, m_handleWidth(7.0)
	, m_minGrid(1)				// Min. space for puzzle table.
{
	initializeGrid(QPointF(0.0, 0.0));
}

void Palapeli::Scene::addPieceToList(Palapeli::Piece* piece)
{
	m_pieces << piece;
}

void Palapeli::Scene::addPieceItemsToScene()
{
	for (Palapeli::Piece * piece : qAsConst(m_pieces)) {
		addItem(piece);
		connect(piece, &Piece::moved, this, &Scene::pieceMoved);
	}
}

void Palapeli::Scene::dispatchPieces(const QList<Palapeli::Piece*> &pieces)
{
	for (Palapeli::Piece * piece : pieces) {
		piece->setSelected(false);
		removeItem(piece);
		m_pieces.removeAll(piece);
		disconnect(piece, &Piece::moved, this, &Scene::pieceMoved);
	}
}

void Palapeli::Scene::clearPieces()
{
	qCDebug(PALAPELI_LOG) << "Palapeli::Scene Delete" << m_pieces.count() << "pieces in  m_pieces list.";
	qDeleteAll(m_pieces);
	qCDebug(PALAPELI_LOG) << "Palapeli::Scene Clear m_pieces list.";
	m_pieces.clear();
	qCDebug(PALAPELI_LOG) << "Palapeli::Scene Stop m_constraintVisualizer.";
	m_constraintVisualizer->stop();
}

void Palapeli::Scene::addMargin(const qreal handleWidth, const qreal spacer) {
	m_handleWidth = handleWidth;
	m_margin = handleWidth + spacer;
	QRectF r = piecesBoundingRect();
	r.adjust(-m_margin, -m_margin, m_margin, m_margin);
	setSceneRect(r);
	m_constraintVisualizer->stop();
	m_constraintVisualizer->start(r, handleWidth);
	views()[0]->fitInView(r, Qt::KeepAspectRatio);
	qCDebug(PALAPELI_LOG) << "SCENE RECT" << r << "VIEW SIZE" << views()[0]->size();
}

QRectF Palapeli::Scene::extPiecesBoundingRect() const
{
	// Bounding rectangle of pieces plus constraint visualizer (margin).
	QRectF result = piecesBoundingRect();
	result.adjust(-m_margin, -m_margin, m_margin, m_margin);
	return result;
}

void Palapeli::Scene::setMinGrid(const int minGrid)
{
	m_minGrid = minGrid;
}

QRectF Palapeli::Scene::piecesBoundingRect() const
{
	// If no pieces, space is >= m_minGrid*m_minGrid pieces (e.g. for a new
	// PieceHolder). Default is >= 1 piece for the puzzle table.
	QRectF result;
	for (Palapeli::Piece* piece : m_pieces)
		result |= piece->sceneBareBoundingRect();
	QSizeF minSize = m_minGrid * m_gridSpacing;
	QRectF minRect(QPointF(0.0, 0.0), minSize);
        if (!m_pieces.isEmpty()) {
		// Center the minRect over the piece(s).
		minRect.moveTopLeft(result.center() - minRect.center());
	}
	return (result | minRect);
}

bool Palapeli::Scene::isConstrained() const
{
	return m_constrained;
}

void Palapeli::Scene::setConstrained(bool constrained)
{
	if (m_constrained == constrained)
		return;
	m_constrained = constrained;
	m_constraintVisualizer->setActive(constrained);
	Q_EMIT constrainedChanged(constrained);
}

void Palapeli::Scene::validatePiecePosition(Palapeli::Piece* piece)
{
	// Get current scene rectangle.
	const QRectF sr = sceneRect();
	// Get bounding rectangle of all pieces and add margin.
	QRectF br = piece->sceneBareBoundingRect(); //br = bounding rect
	br.adjust(-m_margin, -m_margin, m_margin, m_margin);
	if (sr.contains(br))
		return;

	// Check for constraint (ie. pieces must not "push" puzzle table edges).
	if (m_constrained) {
		// Constraint active -> make sure piece stays inside scene rect.
		QPointF pos = piece->pos();
		if (br.left() < sr.left())
			pos.rx() += sr.left() - br.left();
		if (br.right() > sr.right())
			pos.rx() += sr.right() - br.right();
		if (br.top() < sr.top())
			pos.ry() += sr.top() - br.top();
		if (br.bottom() > sr.bottom())
			pos.ry() += sr.bottom() - br.bottom();
		piece->setPos(pos);
	}
	else {
		// Constraint not active -> enlarge scene rect as necessary.
		setSceneRect(sr | br);
	}
}

void Palapeli::Scene::mergeLoadedPieces()
{
	// After loading, merge previously assembled pieces, with no animation.
	// We need to check all the loaded atomic pieces in each scene.
	searchConnections(m_pieces, false);
}

void Palapeli::Scene::searchConnections(const QList<Palapeli::Piece*>& pieces,
					const bool animatedMerging)
{
	// Look for pieces that can be joined after moving or loading.
	// If any are found, merge them, with or without animation.
	QList<Palapeli::Piece*> uncheckedPieces(pieces);
	while (!uncheckedPieces.isEmpty())
	{
		Palapeli::Piece* piece = uncheckedPieces.takeFirst();
		const QList<Palapeli::Piece*> pieceGroup =
			Palapeli::MergeGroup::tryGrowMergeGroup(piece);
		for (Palapeli::Piece* checkedPiece : pieceGroup)
			uncheckedPieces.removeAll(checkedPiece);
		if (pieceGroup.size() > 1)
		{
			Palapeli::MergeGroup* mergeGroup =
				new Palapeli::MergeGroup(pieceGroup, this,
					m_pieceAreaSize, animatedMerging);
			connect(mergeGroup, &Palapeli::MergeGroup::pieceInstanceTransaction, this, &Scene::pieceInstanceTransaction);
			mergeGroup->start();
		}
	}
}

void Palapeli::Scene::pieceInstanceTransaction(const QList<Palapeli::Piece*>& deletedPieces, const QList<Palapeli::Piece*>& createdPieces)
{
	// qCDebug(PALAPELI_LOG) << "Scene::pieceInstanceTransaction(delete" << deletedPieces.count() << "add" << createdPieces.count();
	const int oldPieceCount = m_pieces.count();
	dispatchPieces(deletedPieces);
	for (Palapeli::Piece* newPiece : createdPieces)
	{
		addPieceToList (newPiece);
		connect(newPiece, &Piece::moved,
			this, &Scene::pieceMoved);
	}
	// qCDebug(PALAPELI_LOG) << "emit saveMove(" << oldPieceCount - m_pieces.count();
	Q_EMIT saveMove(oldPieceCount - m_pieces.count());
}

void Palapeli::Scene::pieceMoved(bool finished)
{
	if (!finished) {
		Q_EMIT saveMove(0);
		return;
	}
	// int before = m_pieces.count();
	QList<Palapeli::Piece*> mergeCandidates;
	const auto selectedItems = this->selectedItems();
	for (QGraphicsItem* item : selectedItems)
	{
		Palapeli::Piece* piece = Palapeli::Piece::fromSelectedItem(item);
		if (piece)
			mergeCandidates << piece;
	}
	searchConnections(mergeCandidates, true);	// With animation.
}

void Palapeli::Scene::initializeGrid(const QPointF& gridTopLeft)
{
	m_gridTopLeft = gridTopLeft;
	m_gridSpacing = pieceAreaSize()*(1.0 + 0.05 * Settings::pieceSpacing());
	m_gridRank = 1;
	m_gridX = 0;
	m_gridY = 0;
	// qCDebug(PALAPELI_LOG) << "GRID INITIALIZED" << m_gridTopLeft
	//	 << "spacing" << m_gridSpacing << "scene size" << sceneRect();
}

void Palapeli::Scene::addToGrid(Palapeli::Piece* piece)
{
	// qCDebug(PALAPELI_LOG) << "ADD TO GRID AT" << m_gridTopLeft
	//	 << QPoint(m_gridX, m_gridY)
	//	 << "spacing" << m_gridSpacing << "random" << false;
	piece->setPlace(m_gridTopLeft, m_gridX, m_gridY, m_gridSpacing, false);
	// Calculate the next spot on the square grid.
	if (m_gridY == (m_gridRank - 1)) {
		m_gridX++;		// Add to bottom row.
		if (m_gridX > (m_gridRank - 1)) {
			m_gridRank++;	// Expand the square grid.
			m_gridX = m_gridRank - 1;
			m_gridY = 0;	// Start right-hand column.
		}
	}
	else {
		m_gridY++;		// Add to right-hand column.
		if (m_gridY == (m_gridRank - 1)) {
			m_gridX = 0;	// Start bottom row.
		}
	}
}



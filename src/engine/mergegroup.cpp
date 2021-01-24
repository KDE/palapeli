/*
    SPDX-FileCopyrightText: 2009, 2010 Stefan Majewsky <majewsky@gmx.net>
    SPDX-FileCopyrightText: 2010 Johannes Löhnert <loehnert.kde@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mergegroup.h"
#include "piece.h"
#include "settings.h"

#include <QGraphicsScene>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QSet>

static bool arePiecesPhysicallyNeighboring(Palapeli::Piece* piece1, Palapeli::Piece* piece2)
{
	const qreal snappingPrecision = qreal(Settings::snappingPrecision()) / 100.0;
	const QSizeF snappingSize = snappingPrecision * piece1->atomicSize().expandedTo(piece2->atomicSize());
	const QPointF posDifference = piece1->pos() - piece2->pos();
	return qAbs(posDifference.x()) <= snappingSize.width() && qAbs(posDifference.y()) <= snappingSize.height();
}

QList<Palapeli::Piece*> Palapeli::MergeGroup::tryGrowMergeGroup(Palapeli::Piece* piece)
{
	//definitions in this context:
	//   logical neighbors = two pieces that occupy adjacent positions in the result image
	//   physical neighbors = logical neighbors that have very similar coordinate systems in the current system
	QList<Palapeli::Piece*> resultList;
	QSet<Palapeli::Piece*> resultSet;
	resultList << piece; //this is our return value
	resultSet << piece;  //this is for fast contains() checks
	bool addedSomethingInThisLoop = false;
	do
	{
		addedSomethingInThisLoop = false; //not yet
		//check all neighbors of the currently included pieces
		const auto currentResultList = resultList;
		for (Palapeli::Piece* piece : currentResultList)
		{
			const auto logicalNeighbors = piece->logicalNeighbors();
			for (Palapeli::Piece* logicalNeighbor : logicalNeighbors)
			{
				if (piece->scene() != logicalNeighbor->scene())
					continue;
				//no need to check the located physical neighbors again
				if (resultSet.contains(logicalNeighbor))
					continue;
				if (arePiecesPhysicallyNeighboring(piece, logicalNeighbor))
				{
					resultList << logicalNeighbor;
					resultSet << logicalNeighbor;
					addedSomethingInThisLoop = true;
				}
			}
		}
	}
	while (addedSomethingInThisLoop);
	return resultList;
}

Palapeli::MergeGroup::MergeGroup(const QList<Palapeli::Piece*>& pieces, QGraphicsScene* scene, const QSizeF& pieceAreaSize, bool animated)
	: m_animated(animated)
	, m_pieces(pieces)
	, m_mergedPiece(nullptr)
	, m_scene(scene)
	, m_pieceAreaSize(pieceAreaSize)
{
	//find united coordinate system (UCS) -> large pieces contribute more than smaller pieces
	int totalWeight = 0;
	for (Palapeli::Piece* piece : pieces)
	{
		const int weight = piece->representedAtomicPieces().count();
		m_ucsPosition += weight * piece->pos();
		totalWeight += weight;
	}
	m_ucsPosition /= totalWeight;
}

void Palapeli::MergeGroup::start()
{
	//if no animation is needed, continue directly
	if (!m_animated)
		createMergedPiece();
	else
	{
		//create animations for merging the piece coordinate systems into the UCS
		QParallelAnimationGroup* masterAnimator = new QParallelAnimationGroup(this);
		for (Palapeli::Piece* piece : qAsConst(m_pieces))
		{
			QPropertyAnimation* pieceAnimator = new QPropertyAnimation(piece, "pos", nullptr);
			pieceAnimator->setStartValue(piece->pos());
			pieceAnimator->setEndValue(m_ucsPosition);
			pieceAnimator->setDuration(200);
			pieceAnimator->setEasingCurve(QEasingCurve::InCubic);
			masterAnimator->addAnimation(pieceAnimator);
		}
		masterAnimator->start(QAbstractAnimation::DeleteWhenStopped);
		connect(masterAnimator, &QParallelAnimationGroup::finished, this, &MergeGroup::createMergedPiece);
	}
}

Palapeli::Piece* Palapeli::MergeGroup::mergedPiece() const
{
	return m_mergedPiece;
}

void Palapeli::MergeGroup::createMergedPiece()
{
	//collect pixmaps for merging (also shadows if possible)
	QList<Palapeli::PieceVisuals> pieceVisuals;
	QList<Palapeli::PieceVisuals> shadowVisuals;
	QList<Palapeli::PieceVisuals> highlightVisuals;
	bool allPiecesHaveShadows = true;
	for (Palapeli::Piece* piece : qAsConst(m_pieces))
	{
		pieceVisuals << piece->pieceVisuals();
		if (allPiecesHaveShadows) //we stop collecting shadow samples when one piece has no shadow
		{
			const Palapeli::PieceVisuals shadowSample = piece->shadowVisuals();
			if (shadowSample.isNull())
				allPiecesHaveShadows = false;
			else
				shadowVisuals << shadowSample;
		}
		// Single pieces are assigned highlight items lazily (i.e. if
		// they happen to get selected), but when they are merged, each
		// one must have a highlight pixmap that can be merged into a
		// combined highlight pixmap for the new multi-part piece.
		if (!piece->hasHighlight()) {
			piece->createHighlight(m_pieceAreaSize);
		}
		highlightVisuals << piece->highlightVisuals();
	}
	//merge pixmap and create piece
	Palapeli::PieceVisuals combinedPieceVisuals = Palapeli::mergeVisuals(pieceVisuals);
	Palapeli::PieceVisuals combinedShadowVisuals, combinedHighlightVisuals;
	if (allPiecesHaveShadows)
		combinedShadowVisuals = Palapeli::mergeVisuals(shadowVisuals);
	combinedHighlightVisuals = Palapeli::mergeVisuals(highlightVisuals);
	m_mergedPiece = new Palapeli::Piece(combinedPieceVisuals,
			combinedShadowVisuals, combinedHighlightVisuals);
	//apply UCS
	if (m_animated) {	// If loading the scene, we add the piece later.
		m_scene->addItem(m_mergedPiece);
	}
	m_mergedPiece->setPos(m_ucsPosition);
	//transfer information from old pieces to new piece, then destroy old pieces
	for (Palapeli::Piece* piece : qAsConst(m_pieces))
	{
		m_mergedPiece->addRepresentedAtomicPieces(piece->representedAtomicPieces());
		m_mergedPiece->addLogicalNeighbors(piece->logicalNeighbors());
		m_mergedPiece->addAtomicSize(piece->atomicSize());
		if (piece->isSelected())
			m_mergedPiece->setSelected(true);
		m_mergedPiece->setZValue(qMax(m_mergedPiece->zValue(), piece->zValue()));
		piece->announceReplaced(m_mergedPiece); //make sure that interactors know about the change, and delete the piece
	}
	m_mergedPiece->rewriteLogicalNeighbors(m_pieces, nullptr); //0 = these neighbors should be dropped
	const auto logicalNeighbors = m_mergedPiece->logicalNeighbors();
	for (Palapeli::Piece* logicalNeighbor : logicalNeighbors)
		logicalNeighbor->rewriteLogicalNeighbors(m_pieces, m_mergedPiece); //these neighbors are now represented by m_mergedPiece
	//transaction done
	Q_EMIT pieceInstanceTransaction(m_pieces, QList<Palapeli::Piece*>() << m_mergedPiece);

	// Do not highlight the merged piece, esp. not the solution-in-progress.
	m_mergedPiece->setSelected(false);
	deleteLater();
}



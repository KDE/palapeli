/***************************************************************************
 *   Copyright 2009, 2010 Stefan Majewsky <majewsky@gmx.net>
 *   Copyright 2010 Johannes Löhnert <loehnert.kde@gmx.de>
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
		foreach (Palapeli::Piece* piece, resultList)
		{
			foreach (Palapeli::Piece* logicalNeighbor, piece->logicalNeighbors())
			{
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

Palapeli::MergeGroup::MergeGroup(const QList<Palapeli::Piece*>& pieces, QGraphicsScene* scene, bool animated)
	: m_animated(animated)
	, m_pieces(pieces)
	, m_mergedPiece(0)
	, m_scene(scene)
{
	//find united coordinate system (UCS) -> large pieces contribute more than smaller pieces
	int totalWeight = 0;
	foreach (Palapeli::Piece* piece, pieces)
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
		foreach (Palapeli::Piece* piece, m_pieces)
		{
			QPropertyAnimation* pieceAnimator = new QPropertyAnimation(piece, "pos", 0);
			pieceAnimator->setStartValue(piece->pos());
			pieceAnimator->setEndValue(m_ucsPosition);
			pieceAnimator->setDuration(200);
			pieceAnimator->setEasingCurve(QEasingCurve::InCubic);
			masterAnimator->addAnimation(pieceAnimator);
		}
		masterAnimator->start(QAbstractAnimation::DeleteWhenStopped);
		connect(masterAnimator, SIGNAL(finished()), this, SLOT(createMergedPiece()));
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
	foreach (Palapeli::Piece* piece, m_pieces)
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
		if (!piece->hasHighlight())
			piece->createHighlight();
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
	m_scene->addItem(m_mergedPiece);
	m_mergedPiece->setPos(m_ucsPosition);
	//transfer information from old pieces to new piece, then destroy old pieces
	foreach (Palapeli::Piece* piece, m_pieces)
	{
		m_mergedPiece->addRepresentedAtomicPieces(piece->representedAtomicPieces());
		m_mergedPiece->addLogicalNeighbors(piece->logicalNeighbors());
		m_mergedPiece->addAtomicSize(piece->atomicSize());
		if (piece->isSelected())
			m_mergedPiece->setSelected(true);
		m_mergedPiece->setZValue(qMax(m_mergedPiece->zValue(), piece->zValue()));
		piece->announceReplaced(m_mergedPiece); //make sure that interactors know about the change, and delete the piece
	}
	m_mergedPiece->rewriteLogicalNeighbors(m_pieces, 0); //0 = these neighbors should be dropped
	foreach (Palapeli::Piece* logicalNeighbor, m_mergedPiece->logicalNeighbors())
		logicalNeighbor->rewriteLogicalNeighbors(m_pieces, m_mergedPiece); //these neighbors are now represented by m_mergedPiece
	//transaction done
	emit pieceInstanceTransaction(m_pieces, QList<Palapeli::Piece*>() << m_mergedPiece);
	deleteLater();
}

#include "mergegroup.moc"

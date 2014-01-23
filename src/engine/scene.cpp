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

#include "scene.h"
#include "constraintvisualizer.h"
#include "mergegroup.h"
#include "piece.h"

#include <QGraphicsView>
#include <QDebug>

Palapeli::Scene::Scene(QObject* parent)
	: QGraphicsScene(parent)
	, m_constrained(false)
	, m_constraintVisualizer(new Palapeli::ConstraintVisualizer(this))
	, m_puzzle(0)
	, m_pieceAreaSize(QSizeF())
	, m_margin(10.0)
{
}

void Palapeli::Scene::addPiece(Palapeli::Piece* piece)
{
	addItem(piece);
	m_pieces << piece;
	connect(piece, SIGNAL(moved(bool)), this, SLOT(pieceMoved(bool)));
}

void Palapeli::Scene::clearPieces()
{
	qDeleteAll(m_pieces);
	m_pieces.clear();
}

void Palapeli::Scene::addMargin(const qreal handleWidth, const qreal spacer) {
	m_margin = handleWidth + spacer;
	m_constraintVisualizer->setThickness(handleWidth);
	QRectF r = piecesBoundingRect();
	r.adjust(-m_margin, -m_margin, m_margin, m_margin);
	setSceneRect(r);
}

void Palapeli::Scene::startPuzzle()
{
	emit puzzleStarted();
}

QRectF Palapeli::Scene::piecesBoundingRect() const
{
	QRectF result;
	foreach (Palapeli::Piece* piece, m_pieces)
		result |= piece->sceneBareBoundingRect();
	return result;
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
	emit constrainedChanged(constrained);
}

void Palapeli::Scene::validatePiecePosition(Palapeli::Piece* piece)
{
	//get system geometry
	const QRectF sr = sceneRect();
	const QRectF br = piece->sceneBareBoundingRect(); //br = bounding rect
	if (sr.contains(br))
		return;
	//check constraint
	if (m_constrained)
	{
		QPointF pos = piece->pos();
		//scene rect constraint is active -> ensure that piece stays inside scene rect
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
	else
		//scene rect constraint is not active -> enlarge scene rect as necessary
		setSceneRect(sr | br);
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
		foreach (Palapeli::Piece* checkedPiece, pieceGroup)
			uncheckedPieces.removeAll(checkedPiece);
		if (pieceGroup.size() > 1)
		{
			Palapeli::MergeGroup* mergeGroup =
				new Palapeli::MergeGroup(pieceGroup, this,
							 animatedMerging);
			connect(mergeGroup, SIGNAL(pieceInstanceTransaction(
			    QList<Palapeli::Piece*>,QList<Palapeli::Piece*>)),
			    this, SLOT(pieceInstanceTransaction(
			    QList<Palapeli::Piece*>,QList<Palapeli::Piece*>)));
			mergeGroup->start();
		}
	}
}

void Palapeli::Scene::pieceInstanceTransaction(const QList<Palapeli::Piece*>& deletedPieces, const QList<Palapeli::Piece*>& createdPieces)
{
	qDebug() << "Scene::pieceInstanceTransaction(delete" << deletedPieces.count() << "add" << createdPieces.count();
	const int oldPieceCount = m_pieces.count();
	foreach (Palapeli::Piece* oldPiece, deletedPieces)
		m_pieces.removeAll(oldPiece); //these pieces have been deleted by the caller
	foreach (Palapeli::Piece* newPiece, createdPieces)
	{
		m_pieces << newPiece;
		connect(newPiece, SIGNAL(moved(bool)),
			this, SLOT(pieceMoved(bool)));
	}
	qDebug() << "emit saveMove(" << oldPieceCount - m_pieces.count();
	emit saveMove(oldPieceCount - m_pieces.count());
}

void Palapeli::Scene::pieceMoved(bool finished)
{
	if (!finished) {
		emit saveMove(0);
		return;
	}
	int before = m_pieces.count();
	QList<Palapeli::Piece*> mergeCandidates;
	foreach (QGraphicsItem* item, selectedItems())
	{
		Palapeli::Piece* piece = Palapeli::Piece::fromSelectedItem(item);
		if (piece)
			mergeCandidates << piece;
	}
	searchConnections(mergeCandidates, true);	// With animation.
}

#include "scene.moc"

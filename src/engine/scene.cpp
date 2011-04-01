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
#include "settings.h"
#include "../file-io/components.h"
#include "../file-io/puzzle.h"

#include <cmath>
#include <QFile>
#include <QFutureWatcher>
#include <QGraphicsView>
#include <QPropertyAnimation>
#include <QTimer>
#include <QtCore/qmath.h>
#include <KConfig>
#include <KConfigGroup>
#include <KMessageBox>
#include <KLocalizedString>
#include <KStandardDirs>

typedef QPair<int, int> DoubleIntPair; //comma in type is not possible in foreach macro

Palapeli::Scene::Scene(QObject* parent)
	: QGraphicsScene(parent)
	, m_constrained(false)
	, m_constraintVisualizer(new Palapeli::ConstraintVisualizer(this))
	, m_puzzle(0)
	, m_savegameTimer(new QTimer(this))
	, m_loadingPuzzle(false)
{
	m_savegameTimer->setInterval(500); //write savegame twice per second at most
	m_savegameTimer->setSingleShot(true);
	connect(m_savegameTimer, SIGNAL(timeout()), this, SLOT(updateSavegame()));
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

void Palapeli::Scene::searchConnections(const QList<Palapeli::Piece*>& pieces)
{
	QList<Palapeli::Piece*> uncheckedPieces(pieces);
	const bool animatedMerging = !m_loadingPuzzle;
	while (!uncheckedPieces.isEmpty())
	{
		Palapeli::Piece* piece = uncheckedPieces.takeFirst();
		const QList<Palapeli::Piece*> pieceGroup = Palapeli::MergeGroup::tryGrowMergeGroup(piece);
		foreach (Palapeli::Piece* checkedPiece, pieceGroup)
			uncheckedPieces.removeAll(checkedPiece);
		if (pieceGroup.size() > 1)
		{
			Palapeli::MergeGroup* mergeGroup = new Palapeli::MergeGroup(pieceGroup, this, animatedMerging);
			connect(mergeGroup, SIGNAL(pieceInstanceTransaction(const QList<Palapeli::Piece*>&, const QList<Palapeli::Piece*>&)), this, SLOT(pieceInstanceTransaction(const QList<Palapeli::Piece*>&, const QList<Palapeli::Piece*>&)));
			mergeGroup->start();
		}
	}
}

void Palapeli::Scene::pieceInstanceTransaction(const QList<Palapeli::Piece*>& deletedPieces, const QList<Palapeli::Piece*>& createdPieces)
{
	const int oldPieceCount = m_pieces.count();
	foreach (Palapeli::Piece* oldPiece, deletedPieces)
		m_pieces.removeAll(oldPiece); //these pieces have been deleted by the caller
	foreach (Palapeli::Piece* newPiece, createdPieces)
	{
		m_pieces << newPiece;
		connect(newPiece, SIGNAL(moved()), this, SLOT(pieceMoved()));
	}
	if (!m_loadingPuzzle)
	{
		emit reportProgress(m_atomicPieceCount, m_pieces.count());
		//victory animation
		if (m_pieces.count() == 1 && oldPieceCount > 1)
			QTimer::singleShot(0, this, SLOT(playVictoryAnimation()));
	}
}

void Palapeli::Scene::loadPuzzle(Palapeli::Puzzle* puzzle)
{
	if (m_loadingPuzzle)
		return;
	//load puzzle
	if (puzzle && m_puzzle != puzzle)
	{
		m_puzzle = puzzle;
		loadPuzzleInternal();
	}
}

void Palapeli::Scene::loadPuzzleInternal()
{
	m_loadingPuzzle = true;
	//reset behavioral parameters
	setConstrained(false);
	//clear scene
	qDeleteAll(m_pieces); m_pieces.clear();
	emit reportProgress(0, 0);
	//begin to load puzzle
	m_loadedPieces.clear();
	if (m_puzzle)
	{
		Palapeli::FutureWatcher* watcher = new Palapeli::FutureWatcher;
		connect(watcher, SIGNAL(finished()), SLOT(loadNextPiece()));
		connect(watcher, SIGNAL(finished()), watcher, SLOT(deleteLater()));
		watcher->setFuture(m_puzzle->get(Palapeli::PuzzleComponent::Contents));
	}
}

void Palapeli::Scene::loadNextPiece()
{
	if (!m_puzzle)
		return;
	const Palapeli::ContentsComponent* component = m_puzzle->component<Palapeli::ContentsComponent>();
	if (!component)
		return;
	//add pieces, but only one at a time
	const Palapeli::PuzzleContents contents = component->contents;
	QMap<int, QImage>::const_iterator iterPieces = contents.pieces.begin();
	const QMap<int, QImage>::const_iterator iterPiecesEnd = contents.pieces.end();
	for (int pieceID = iterPieces.key(); iterPieces != iterPiecesEnd; pieceID = (++iterPieces).key())
	{
		if (m_loadedPieces.contains(pieceID))
			continue; //already loaded
		//load piece
		Palapeli::Piece* piece = new Palapeli::Piece(iterPieces.value(), contents.pieceOffsets[pieceID]);
		piece->addRepresentedAtomicPieces(QList<int>() << pieceID);
		piece->addAtomicSize(iterPieces.value().size());
		addItem(piece);
		m_pieces << piece;
		m_loadedPieces[pieceID] = piece;
		connect(piece, SIGNAL(moved()), this, SLOT(pieceMoved()));
		//continue with next piece after eventloop run
		if (contents.pieces.size() > m_pieces.size())
			QTimer::singleShot(0, this, SLOT(loadNextPiece()));
		else
			QTimer::singleShot(0, this, SLOT(loadPiecePositions()));
		return;
	}
}

void Palapeli::Scene::loadPiecePositions()
{
	if (!m_puzzle)
		return;
	const Palapeli::PuzzleContents contents = m_puzzle->component<Palapeli::ContentsComponent>()->contents;
	//add piece relations
	foreach (const DoubleIntPair& relation, contents.relations)
	{
		Palapeli::Piece* firstPiece = m_pieces[relation.first];
		Palapeli::Piece* secondPiece = m_pieces[relation.second];
		firstPiece->addLogicalNeighbors(QList<Palapeli::Piece*>() << secondPiece);
		secondPiece->addLogicalNeighbors(QList<Palapeli::Piece*>() << firstPiece);
	}
	//Is "savegame" available?
	static const QString pathTemplate = QString::fromLatin1("collection/%1.save");
	KConfig saveConfig(KStandardDirs::locateLocal("appdata", pathTemplate.arg(m_puzzle->identifier())));
	if (saveConfig.hasGroup("SaveGame"))
	{
		//read piece positions from savegame
		KConfigGroup saveGroup(&saveConfig, "SaveGame");
		QMap<int, Palapeli::Piece*>::const_iterator iterPieces = m_loadedPieces.constBegin();
		const QMap<int, Palapeli::Piece*>::const_iterator iterPiecesEnd = m_loadedPieces.constEnd();
		for (int pieceID = iterPieces.key(); iterPieces != iterPiecesEnd; pieceID = (++iterPieces).key())
		{
			Palapeli::Piece* piece = iterPieces.value();
			piece->setPos(saveGroup.readEntry(QString::number(pieceID), QPointF()));
		}
		searchConnections(m_pieces);
	}
	else
	{
		//place pieces at nice positions
		//step 1: determine maximum piece size
		QSizeF pieceAreaSize;
		foreach (Palapeli::Piece* piece, m_pieces)
			pieceAreaSize = pieceAreaSize.expandedTo(piece->sceneBareBoundingRect().size());
		pieceAreaSize *= 1.3; //more space for each piece
		//step 2: place pieces in a grid in random order
		QList<Palapeli::Piece*> piecePool(m_pieces);
		const int xCount = floor(qSqrt(piecePool.count()));
		for (int y = 0; !piecePool.isEmpty(); ++y)
		{
			for (int x = 0; x < xCount && !piecePool.isEmpty(); ++x)
			{
				//select random piece
				Palapeli::Piece* piece = piecePool.takeAt(qrand() % piecePool.count());
				//determine piece offset
				piece->setPos(QPointF());
				const QRectF br = piece->sceneBareBoundingRect();
				const QPointF pieceOffset = br.topLeft();
				const QSizeF pieceSize = br.size();
				//determine random position inside piece area
				const QPointF areaOffset(
					qrand() % (int)(pieceAreaSize.width() - pieceSize.width()),
					qrand() % (int)(pieceAreaSize.height() - pieceSize.height())
				);
				//move to desired position in (x,y) grid
				const QPointF gridBasePosition(x * pieceAreaSize.width(), y * pieceAreaSize.height());
				piece->setPos(gridBasePosition + areaOffset - pieceOffset);
			}
		}
	}
	//continue after eventloop run
	QTimer::singleShot(0, this, SLOT(completeVisualsForNextPiece()));
}

void Palapeli::Scene::completeVisualsForNextPiece()
{
	foreach (Palapeli::Piece* piece, m_pieces)
	{
		if (piece->completeVisuals())
		{
			//something had to be done -> continue with next piece after eventloop run
			QTimer::singleShot(0, this, SLOT(completeVisualsForNextPiece()));
			return;
		}
	}
	//no pieces without shadow left, or piece visuals completely disabled
	finishLoading();
}

void Palapeli::Scene::finishLoading()
{
	m_puzzle->dropComponent(Palapeli::PuzzleComponent::Contents);
	//determine scene rect
	setSceneRect(piecesBoundingRect());
	//initialize external progress display
	m_atomicPieceCount = m_loadedPieces.count();
	emit reportProgress(m_atomicPieceCount, m_pieces.count());
	emit puzzleStarted();
	m_loadingPuzzle = false;
	//check if puzzle has been completed
	if (m_pieces.count() == 1)
	{
		int result = KMessageBox::questionYesNo(views()[0], i18n("You have finished the puzzle the last time. Do you want to restart it now?"));
		if (result == KMessageBox::Yes)
			restartPuzzle();
	}
}

void Palapeli::Scene::pieceMoved()
{
	QList<Palapeli::Piece*> mergeCandidates;
	foreach (QGraphicsItem* item, selectedItems())
	{
		Palapeli::Piece* piece = Palapeli::Piece::fromSelectedItem(item);
		if (piece)
			mergeCandidates << piece;
	}
	searchConnections(mergeCandidates);
	invalidateSavegame();
	emit reportProgress(m_atomicPieceCount, m_pieces.count());
}

void Palapeli::Scene::invalidateSavegame()
{
	if (!m_savegameTimer->isActive())
		m_savegameTimer->start();
}

void Palapeli::Scene::updateSavegame()
{
	//save piece positions
	static const QString pathTemplate = QString::fromLatin1("collection/%1.save");
	KConfig saveConfig(KStandardDirs::locateLocal("appdata", pathTemplate.arg(m_puzzle->identifier())));
	KConfigGroup saveGroup(&saveConfig, "SaveGame");
	foreach (Palapeli::Piece* piece, m_pieces)
	{
		const QPointF pos = piece->pos();
		foreach (int atomicPieceID, piece->representedAtomicPieces())
			saveGroup.writeEntry(QString::number(atomicPieceID), pos);
	}
}

void Palapeli::Scene::playVictoryAnimation()
{
	setConstrained(true);
	QPropertyAnimation* animation = new QPropertyAnimation(this, "sceneRect", this);
	animation->setStartValue(sceneRect());
	animation->setEndValue(piecesBoundingRect());
	animation->setDuration(1000);
	connect(animation, SIGNAL(finished()), this, SLOT(playVictoryAnimation2()));
	animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void Palapeli::Scene::playVictoryAnimation2()
{
	setSceneRect(piecesBoundingRect());
	QTimer::singleShot(100, this, SIGNAL(victoryAnimationFinished()));
	QTimer::singleShot(1500, this, SLOT(playVictoryAnimation3())); //give the View some time to play its part of the victory animation
}

void Palapeli::Scene::playVictoryAnimation3()
{
	KMessageBox::information(views()[0], i18n("Great! You have finished the puzzle."));
}

void Palapeli::Scene::restartPuzzle()
{
	static const QString pathTemplate = QString::fromLatin1("collection/%1.save");
	QFile(KStandardDirs::locateLocal("appdata", pathTemplate.arg(m_puzzle->identifier()))).remove();
	//reload puzzle
	loadPuzzleInternal();
}

#include "scene.moc"

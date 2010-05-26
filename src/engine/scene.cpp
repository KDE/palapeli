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

#include "scene.h"
#include "part.h"
#include "piece.h"
#include "../file-io/collection.h"
#include "../file-io/puzzle.h"
#include "settings.h"

#include <cmath>
#include <QFile>
#include <QGraphicsView>
#include <QPropertyAnimation>
#include <QTimer>
#include <QtConcurrentRun>
#include <KConfig>
#include <KConfigGroup>
#include <KMessageBox>
#include <KLocalizedString>
#include <KStandardDirs>

typedef QPair<int, int> DoubleIntPair; //comma in type is not possible in foreach macro

Palapeli::Scene::Scene(QObject* parent)
	: QGraphicsScene(parent)
	, m_constrained(false)
	, m_partGroup(new Palapeli::EmptyGraphicsObject)
	, m_loadingPuzzle(false)
{
	addItem(m_partGroup);
	connect(&m_metadataLoader, SIGNAL(finished()), this, SLOT(continueLoading()));
}

QRectF Palapeli::Scene::partsBoundingRect() const
{
	QRectF result;
	foreach (Palapeli::Part* part, m_parts)
		result |= part->mapToScene(part->piecesBoundingRect()).boundingRect();
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
	emit constrainedChanged(constrained);
}

void Palapeli::Scene::loadPuzzle(const QModelIndex& index)
{
	if (m_loadingPuzzle)
		return;
	//load puzzle
	QObject* puzzlePayload = index.data(Palapeli::Collection::PuzzleObjectRole).value<QObject*>();
	Palapeli::Puzzle* puzzle = qobject_cast<Palapeli::Puzzle*>(puzzlePayload);
	if (puzzle && m_puzzle != puzzle)
	{
		m_puzzle = puzzle;
		m_identifier = index.data(Palapeli::Collection::IdentifierRole).toString();
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
	qDeleteAll(m_parts); m_parts.clear();
	emit reportProgress(0, 0);
	//begin to load puzzle
	startLoading();
}

void Palapeli::Scene::startLoading()
{
	if (m_puzzle)
	{
		if (m_puzzle->metadata())
			continueLoading();
		else
			m_metadataLoader.setFuture(QtConcurrent::run(m_puzzle.data(), &Palapeli::Puzzle::readMetadata, false));
			//will call continueLoading() when done reading metadata
	}
}

void Palapeli::Scene::continueLoading()
{
	if (!m_puzzle)
		return;
	//continue to read puzzle
	if (!m_puzzle->metadata()) //reading the archive has failed
		return;
	if (!m_puzzle->readContents()) //this cannot be done in a separate thread
		return;
	//delay piece loading for UI responsibility
	if (!m_puzzle->contents()->pieces.isEmpty())
		QTimer::singleShot(0, this, SLOT(loadNextPart()));
}

void Palapeli::Scene::loadNextPart()
{
	if (!m_puzzle)
		return;
	//add pieces and parts, but only one piece at a time
	const Palapeli::PuzzleContents* contents = m_puzzle->contents();
	QMap<int, QPixmap>::const_iterator iterPieces = contents->pieces.begin();
	const QMap<int, QPixmap>::const_iterator iterPiecesEnd = contents->pieces.end();
	for (int pieceID = iterPieces.key(); iterPieces != iterPiecesEnd; pieceID = (++iterPieces).key())
	{
		if (m_pieces.contains(pieceID))
			continue; //already loaded
		Palapeli::Piece* piece = new Palapeli::Piece(iterPieces.value(), contents->pieceOffsets[pieceID]);
		m_pieces[pieceID] = piece;
		Palapeli::Part* part = new Palapeli::Part(piece);
		connect(part, SIGNAL(destroyed(QObject*)), this, SLOT(partDestroyed(QObject*)));
		connect(part, SIGNAL(partMoved()), this, SLOT(partMoved()));
		connect(part, SIGNAL(partMoving()), this, SLOT(partMoving()));
		part->setParentItem(m_partGroup);
		m_parts << part;
		//continue with next part after eventloop run
		if (contents->pieces.size() > m_pieces.size())
			QTimer::singleShot(0, this, SLOT(loadNextPart()));
		else
			QTimer::singleShot(0, this, SLOT(finishLoading()));
		return;
	}
}

void Palapeli::Scene::finishLoading()
{
	if (!m_puzzle)
		return;
	const Palapeli::PuzzleContents* contents = m_puzzle->contents();
	//add piece relations
	foreach (const DoubleIntPair& relation, contents->relations)
	{
		Palapeli::Piece* firstPiece = m_pieces[relation.first];
		Palapeli::Piece* secondPiece = m_pieces[relation.second];
		firstPiece->addNeighbor(secondPiece);
		secondPiece->addNeighbor(firstPiece);
	}
	//Is "savegame" available?
	static const QString pathTemplate = QString::fromLatin1("collection/%1.save");
	KConfig saveConfig(KStandardDirs::locateLocal("appdata", pathTemplate.arg(m_identifier)));
	if (saveConfig.hasGroup("SaveGame"))
	{
		//read piece positions from savegame
		KConfigGroup saveGroup(&saveConfig, "SaveGame");
		QMap<int, Palapeli::Piece*>::const_iterator iterPieces = m_pieces.constBegin();
		const QMap<int, Palapeli::Piece*>::const_iterator iterPiecesEnd = m_pieces.constEnd();
		for (int pieceID = iterPieces.key(); iterPieces != iterPiecesEnd; pieceID = (++iterPieces).key())
		{
			Palapeli::Part* part = iterPieces.value()->part();
			part->setPos(saveGroup.readEntry(QString::number(pieceID), QPointF()));
		}
		for (iterPieces = m_pieces.constBegin(); iterPieces != iterPiecesEnd; ++iterPieces)
		{
			Palapeli::Part* part = iterPieces.value()->part();
			part->searchConnections();
			part->validatePosition();
		}
	}
	else
	{
		//place parts at nice positions
		//step 1: determine maximum part size
		QSizeF partAreaSize;
		foreach (Palapeli::Part* part, m_parts)
			partAreaSize = partAreaSize.expandedTo(part->scenePiecesBoundingRect().size());
		partAreaSize *= 1.3; //more space for each part
		//step 2: place parts in a grid in random order
		QList<Palapeli::Part*> partPool(m_parts);
		const int xCount = floor(sqrt((float)partPool.count()));
		for (int y = 0; !partPool.isEmpty(); ++y)
		{
			for (int x = 0; x < xCount && !partPool.isEmpty(); ++x)
			{
				//select random part
				Palapeli::Part* part = partPool.takeAt(qrand() % partPool.count());
				//determine part offset
				part->setPos(QPointF());
				const QRectF br = part->scenePiecesBoundingRect();
				const QPointF partOffset = br.topLeft();
				const QSizeF partSize = br.size();
				//determine random position inside part area
				const QPointF areaOffset(
					qrand() % (int)(partAreaSize.width() - partSize.width()),
					qrand() % (int)(partAreaSize.height() - partSize.height())
				);
				//move to desired position in (x,y) grid
				const QPointF gridBasePosition(x * partAreaSize.width(), y * partAreaSize.height());
				part->setPos(gridBasePosition + areaOffset - partOffset);
			}
		}
	}
	//determine scene rect
	setSceneRect(partsBoundingRect());
	//initialize external progress display
	emit reportProgress(m_pieces.count(), m_parts.count());
	emit puzzleStarted();
	m_loadingPuzzle = false;
	//check if puzzle has been completed
	if (m_parts.count() == 1)
	{
		int result = KMessageBox::questionYesNo(views()[0], i18n("You have finished the puzzle the last time. Do you want to restart it now?"));
		if (result == KMessageBox::Yes)
			restartPuzzle();
	}
}

void Palapeli::Scene::partDestroyed(QObject* object)
{
	int oldCount = m_parts.count();
	m_parts.removeAll(reinterpret_cast<Palapeli::Part*>(object));
	//victory animation
	if (m_parts.count() == 1 && oldCount > 1 && !m_loadingPuzzle)
		QTimer::singleShot(0, this, SLOT(playVictoryAnimation()));
}

void Palapeli::Scene::partMoving()
{
	//if scene size constraint is not active, enlarge scene rect as needed
	if (!m_constrained)
		setSceneRect(partsBoundingRect() | sceneRect());
}

void Palapeli::Scene::partMoved()
{
	partMoving();
	emit reportProgress(m_pieces.count(), m_parts.count());
	//save piece positions
	static const QString pathTemplate = QString::fromLatin1("collection/%1.save");
	KConfig saveConfig(KStandardDirs::locateLocal("appdata", pathTemplate.arg(m_identifier)));
	KConfigGroup saveGroup(&saveConfig, "SaveGame");
	QMap<int, Palapeli::Piece*>::const_iterator iterPieces = m_pieces.constBegin();
	const QMap<int, Palapeli::Piece*>::const_iterator iterPiecesEnd = m_pieces.constEnd();
	for (int pieceID = iterPieces.key(); iterPieces != iterPiecesEnd; pieceID = (++iterPieces).key())
		saveGroup.writeEntry(QString::number(pieceID), iterPieces.value()->part()->pos());
}

void Palapeli::Scene::playVictoryAnimation()
{
	setConstrained(true);
	QPropertyAnimation* animation = new QPropertyAnimation(this, "sceneRect", this);
	animation->setStartValue(sceneRect());
	animation->setEndValue(partsBoundingRect());
	animation->setDuration(1000);
	connect(animation, SIGNAL(finished()), this, SLOT(playVictoryAnimation2()));
	animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void Palapeli::Scene::playVictoryAnimation2()
{
	setSceneRect(partsBoundingRect());
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
	QFile(KStandardDirs::locateLocal("appdata", pathTemplate.arg(m_identifier))).remove();
	//reload puzzle
	loadPuzzleInternal();
}

#include "scene.moc"

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

#include <QFile>
#include <QTimer>
#include <QtConcurrentRun>
#include <KConfig>
#include <KConfigGroup>
#include <KStandardDirs>

typedef QPair<int, int> DoubleIntPair; //comma in type is not possible in foreach macro

Palapeli::Scene::Scene(QObject* parent)
	: QGraphicsScene(parent)
	, m_constrained(false)
	, m_loadingPuzzle(false)
{
	connect(&m_metadataLoader, SIGNAL(finished()), this, SLOT(continueLoading()));
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
	foreach (Palapeli::Part* part, m_parts)
		part->validatePosition();
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
	//reset behavioral parameters
	setConstrained(false);
	//clear scene
	qDeleteAll(m_pieces); m_pieces.clear();
	qDeleteAll(m_parts); m_parts.clear();
	emit reportProgress(0, 0);
	//begin to load puzzle
	m_loadingPuzzle = true;
	startLoading();
}

void Palapeli::Scene::startLoading()
{
	if (m_puzzle)
		m_metadataLoader.setFuture(QtConcurrent::run(m_puzzle.data(), &Palapeli::Puzzle::readMetadata, false));
	//will call continueLoading() when done reading metadata
}

void Palapeli::Scene::continueLoading()
{
	if (!m_puzzle)
		return;
	//continue to read puzzle
	if (m_metadataLoader.future().result() == false) //reading the archive has failed
		return;
	if (!m_puzzle->readContents()) //this cannot be done in a separate thread
		return;
	//initialize scene
	const Palapeli::PuzzleContents* contents = m_puzzle->contents();
	const qreal sceneSizeFactor = 2.5; //TODO: replace this
	setSceneRect(QRectF(QPointF(), sceneSizeFactor * QSizeF(contents->imageSize)));
	//delay piece loading for UI responsibility
	if (!contents->pieces.isEmpty())
		QTimer::singleShot(0, this, SLOT(loadNextPart()));
}

void Palapeli::Scene::loadNextPart()
{
	if (!m_puzzle)
		return;
	//add pieces and parts, but only one piece at a time
	const Palapeli::PuzzleContents* contents = m_puzzle->contents();
	const QList<int> pieceIDs = contents->pieces.keys();
	foreach (int pieceID, pieceIDs)
	{
		if (m_pieces.contains(pieceID))
			continue; //already loaded
		Palapeli::Piece* piece = new Palapeli::Piece(contents->pieces[pieceID], contents->pieceOffsets[pieceID]);
		m_pieces[pieceID] = piece;
		Palapeli::Part* part = new Palapeli::Part(piece);
		addItem(part);
		connect(part, SIGNAL(destroyed(QObject*)), this, SLOT(partDestroyed(QObject*)));
		connect(part, SIGNAL(partMoved()), this, SLOT(partMoved()));
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
	static const QString pathTemplate = QString::fromLatin1("palapeli/puzzlelibrary/%1.save");
	KConfig saveConfig(KStandardDirs::locateLocal("data", pathTemplate.arg(m_identifier)));
	if (saveConfig.hasGroup("SaveGame"))
	{
		//read piece positions from savegame
		KConfigGroup saveGroup(&saveConfig, "SaveGame");
		const QList<int> pieceIDs = m_pieces.keys();
		foreach (int pieceID, pieceIDs)
		{
			Palapeli::Part* part = m_pieces[pieceID]->part();
			part->setPos(saveGroup.readEntry(QString::number(pieceID), QPointF()));
			part->searchConnections();
			part->validatePosition();
		}
		if (!m_constrained)
		{
			//read scene rect from piece positions
			QRectF newSceneRect = sceneRect();
			foreach (Palapeli::Part* part, m_parts)
				newSceneRect = newSceneRect.united(part->mapToScene(part->piecesBoundingRect()).boundingRect());
			setSceneRect(newSceneRect);
		}
	}
	else
	{
		//place parts at random positions (inside the scene rect)
		const QRectF sr = sceneRect();
		foreach (Palapeli::Part* part, m_parts)
		{
			QRectF br = part->sceneTransform().mapRect(part->childrenBoundingRect());
			//NOTE: br = bounding rect (of part), sr = scene rect
			const int minXPos = sr.left(), maxXPos = sr.right() - br.width();
			const int minYPos = sr.top(), maxYPos = sr.center().y() - br.height();
			//NOTE: In the above line, sr.bottom() is replaced by sr.center().y() to only fill half of the table.
			const int xPos = qrand() % (maxXPos - minXPos) + minXPos;
			const int yPos = qrand() % (maxYPos - minYPos) + minYPos;
			part->setPos(xPos - br.left(), yPos - br.top());
		}
	}
	m_loadingPuzzle = false; //finished
	//initialize external progress display
	emit reportProgress(m_pieces.count(), m_parts.count());
}

void Palapeli::Scene::partDestroyed(QObject* object)
{
	m_parts.removeAll(reinterpret_cast<Palapeli::Part*>(object));
}

void Palapeli::Scene::partMoved()
{
	emit reportProgress(m_pieces.count(), m_parts.count());
	//save piece positions
	static const QString pathTemplate = QString::fromLatin1("puzzlelibrary/%1.save");
	KConfig saveConfig(KStandardDirs::locateLocal("appdata", pathTemplate.arg(m_identifier)));
	KConfigGroup saveGroup(&saveConfig, "SaveGame");
	const QList<int> pieceIDs = m_pieces.keys();
	foreach (int pieceID, pieceIDs)
		saveGroup.writeEntry(QString::number(pieceID), m_pieces[pieceID]->part()->pos());
	//if scene size constraint is not active, enlarge scene rect as needed
	if (!m_constrained)
	{
		QRectF newSceneRect = sceneRect();
		foreach (Palapeli::Part* part, m_parts)
			newSceneRect = newSceneRect.united(part->mapToScene(part->piecesBoundingRect()).boundingRect());
		setSceneRect(newSceneRect);
	}
}

void Palapeli::Scene::restartPuzzle()
{
	static const QString pathTemplate = QString::fromLatin1("puzzlelibrary/%1.save");
	QFile(KStandardDirs::locateLocal("appdata", pathTemplate.arg(m_identifier))).remove();
	//reload puzzle
	loadPuzzleInternal();
}

#include "scene.moc"

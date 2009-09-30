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
#include "../file-io/puzzle.h"
#include "settings.h"
#include "shadowitem.h"

#include <QFile>
#include <KConfig>
#include <KConfigGroup>
#include <KStandardDirs>

typedef QPair<int, int> DoubleIntPair; //comma in type is not possible in foreach macro

Palapeli::Scene::Scene(QObject* parent)
	: QGraphicsScene(parent)
{
}

void Palapeli::Scene::loadPuzzle(Palapeli::Puzzle* puzzle)
{
	if (!puzzle->readContents()) //TODO: show error message if failed
		return;
	m_puzzle = puzzle;
	m_identifier = puzzle->location().identifier(); //NOTE: m_identifier is only used for finding the savegame file
	if (!puzzle->location().isFromLibrary())
		m_identifier.prepend(QLatin1String("external-")); //avoid that external puzzles mess up the library savegames
	const Palapeli::PuzzleContents* contents = puzzle->contents();
	//clear scene
	qDeleteAll(m_pieces); m_pieces.clear();
	qDeleteAll(m_parts); m_parts.clear();
	//initialize scene
	setSceneRect(QRectF(QPointF(), Settings::sceneSizeFactor() * QSizeF(contents->imageSize)));
	//add pieces and parts
	QMap<int, Palapeli::Piece*> pieces;
	const QList<int> pieceIDs = contents->pieces.keys();
	foreach (int pieceID, pieceIDs)
	{
		Palapeli::Piece* piece = new Palapeli::Piece(contents->pieces[pieceID], contents->pieceOffsets[pieceID]);
		pieces[pieceID] = piece;
		m_pieces << piece;
		Palapeli::Part* part = new Palapeli::Part(piece);
		addItem(part);
		connect(part, SIGNAL(destroyed(QObject*)), this, SLOT(partDestroyed(QObject*)));
		connect(part, SIGNAL(partMoved()), this, SLOT(partMoved()));
		m_parts << part;
		if (pieceID == 0)
			addItem(new Palapeli::ShadowItem(contents->pieces[0], contents->pieces[0].width() / 10));
	}
	//add piece relations
	foreach (DoubleIntPair relation, contents->relations)
	{
		Palapeli::Piece* firstPiece = pieces[relation.first];
		Palapeli::Piece* secondPiece = pieces[relation.second];
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
		for (int i = 0; i < m_pieces.count(); ++i)
		{
			Palapeli::Part* part = m_pieces[i]->part();
			//TODO: respect sceneRect
			part->setPos(saveGroup.readEntry(QString::number(i), QPointF()));
			part->searchConnections();
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
			const int minYPos = sr.top(), maxYPos = sr.bottom() - br.height();
			const int xPos = qrand() % (maxXPos - minXPos) + minXPos;
			const int yPos = qrand() % (maxYPos - minYPos) + minYPos;
			part->setPos(xPos - br.left(), yPos - br.top());
		}
	}
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
	for (int i = 0; i < m_pieces.count(); ++i)
		saveGroup.writeEntry(QString::number(i), m_pieces[i]->part()->pos());
}

void Palapeli::Scene::restartPuzzle()
{
	static const QString pathTemplate = QString::fromLatin1("puzzlelibrary/%1.save");
	QFile(KStandardDirs::locateLocal("appdata", pathTemplate.arg(m_identifier))).remove();
	//reload puzzle
	loadPuzzle(m_puzzle);
}

#include "scene.moc"

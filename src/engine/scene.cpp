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

typedef QPair<int, int> DoubleIntPair; //comma in type is not possible in foreach macro

Palapeli::Scene::Scene(QObject* parent)
	: QGraphicsScene(parent)
{
}

void Palapeli::Scene::loadPuzzle(Palapeli::Puzzle* puzzle)
{
	//clear scene
	qDeleteAll(m_parts);
	m_parts.clear();
	//initialize scene
	const int sceneSizeFactor = 2;
	setSceneRect(QRectF(QPointF(), sceneSizeFactor * puzzle->imageSize()));
	//add pieces and parts
	QMap<int, Palapeli::Piece*> pieces;
	const QMap<int, QPixmap> pieceImages = puzzle->pieces();
	const QMap<int, QPoint> pieceOffsets = puzzle->pieceOffsets();
	const QList<int> pieceIDs = pieceImages.keys();
	foreach (int pieceID, pieceIDs)
	{
		Palapeli::Piece* piece = new Palapeli::Piece(pieceImages[pieceID], pieceOffsets[pieceID]);
		pieces[pieceID] = piece;
		Palapeli::Part* part = new Palapeli::Part(piece);
		addItem(part);
		connect(part, SIGNAL(destroyed(QObject*)), this, SLOT(partDestroyed(QObject*)));
		connect(part, SIGNAL(partMoved()), this, SLOT(partMoved()));
		m_parts << part;
	}
	//add piece relations
	const QList<DoubleIntPair> relations = puzzle->relations();
	foreach (DoubleIntPair relation, relations)
	{
		Palapeli::Piece* firstPiece = pieces[relation.first];
		Palapeli::Piece* secondPiece = pieces[relation.second];
		firstPiece->addNeighbor(secondPiece);
		secondPiece->addNeighbor(firstPiece);
	}
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

void Palapeli::Scene::partDestroyed(QObject* object)
{
	m_parts.removeAll(reinterpret_cast<Palapeli::Part*>(object));
}

void Palapeli::Scene::partMoved()
{
	//TODO: Implement saving (and also loading) of piece positions.
}

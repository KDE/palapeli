/***************************************************************************
 *   Copyright (C) 2008 Felix Lemke <lemke.felix@ages-skripte.org>
 *   Copyright (C) 2008 Stefan Majewsky <majewsky@gmx.net>
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
#include "minimap.h"
#include "part.h"
#include "piece.h"

#include <QImage>
#include <QPainter>
#include <QPixmap>

Palapeli::Scene::Scene(int width, int height)
	: QGraphicsScene()
{
	setSceneRect(0, 0, width, height);
	m_visualSceneBoundary = addRect(-1.0, -1.0, width + 2.0, height + 2.0);
	m_visualSceneBoundary->setAcceptedMouseButtons(Qt::NoButton);
}

Palapeli::Scene::~Scene()
{
	foreach (Palapeli::Piece* piece, m_pieces)
		delete piece;
	foreach (Palapeli::Part *part, m_parts)
		delete part;
}

QListIterator<Palapeli::Piece*> Palapeli::Scene::pieces() const
{
	return QListIterator<Palapeli::Piece*>(m_pieces);
}

void Palapeli::Scene::loadImage(const QImage& image, int xPieces, int yPieces)
{
	int width = image.width(), height = image.height();
	int pieceWidth = width / xPieces, pieceHeight = height / yPieces;
	int sceneWidth = this->width(), sceneHeight = this->height();
	//make pieces
	for (int x = 0; x < xPieces; ++x)
	{
		for (int y = 0; y < yPieces; ++y)
		{
			QPixmap pix(pieceWidth, pieceHeight);
			QPainter painter(&pix);
			painter.drawImage(QPoint(0, 0), image, QRect(x * pieceWidth, y * pieceHeight, pieceWidth, pieceHeight));
			painter.end();
			Palapeli::Piece* piece = new Palapeli::Piece(pix, this, pieceWidth, pieceHeight);
			piece->setPos(qrand() % (sceneWidth - pieceWidth), qrand() % (sceneHeight - pieceHeight));
			m_pieces << piece;
			Palapeli::Part* part = new Palapeli::Part(piece, this);
			m_parts << part;
			connect(part, SIGNAL(positionsUpdated()), this, SIGNAL(minimapNeedsUpdate()));
		}
	}
	//build relationships between pieces
	for (int x = 0; x < xPieces; ++x)
	{
		for (int y = 0; y < yPieces; ++y)
		{
			//left
			if (x != 0)
				m_pieces[x * yPieces + y]->addNeighbor(m_pieces[(x - 1) * yPieces + y], -pieceWidth, 0);
			//right
			if (x != xPieces - 1)
				m_pieces[x * yPieces + y]->addNeighbor(m_pieces[(x + 1) * yPieces + y], pieceWidth, 0);
			//top
			if (y != 0)
				m_pieces[x * yPieces + y]->addNeighbor(m_pieces[x * yPieces + (y - 1)], 0, -pieceHeight);
			//bottom
			if (y != yPieces - 1)
				m_pieces[x * yPieces + y]->addNeighbor(m_pieces[x * yPieces + (y + 1)], 0, pieceHeight);
		}
	}
}

void Palapeli::Scene::combineParts(Palapeli::Part* part1, Palapeli::Part* part2, qreal dx, qreal dy) //friend of Palapeli::Part
{
	if (!m_parts.contains(part1) || !m_parts.contains(part2))
		return;
	foreach (Palapeli::Piece *piece, part2->m_pieces)
	{
		part1->addPiece(piece);
		piece->moveBy(dx, dy);
	}
	m_parts.removeAll(part2);
	delete part2;
}

#include "scene.moc"

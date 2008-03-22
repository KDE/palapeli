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
#include "part.h"
#include "piece.h"

#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <KDebug>

Palapeli::Scene::Scene(int width, int height)
	: QGraphicsScene()
	, m_xPieces(0)
	, m_yPieces(0)
	, m_pieces(0)
{
	setSceneRect(0, 0, width, height);
	m_visualSceneBoundary = addRect(-1.0, -1.0, width + 2.0, height + 2.0);
	m_visualSceneBoundary->setAcceptedMouseButtons(Qt::NoButton);
}

Palapeli::Scene::~Scene()
{
	for (int x = 0; x < m_xPieces; ++x)
		delete[] m_pieces[x];
	delete[] m_pieces;
	foreach (Palapeli::Part *part, m_parts)
		delete part;
}

void Palapeli::Scene::loadImage(const QString &fileName, int xPieces, int yPieces)
{
	m_xPieces = xPieces;
	m_yPieces = yPieces;
	QImage image(fileName);
	int width = image.width(), height = image.height();
	int pieceWidth = width / xPieces, pieceHeight = height / yPieces;
	int sceneWidth = this->width(), sceneHeight = this->height();
	m_pieces = new Palapeli::Piece**[xPieces];
	for (int x = 0; x < xPieces; ++x)
	{
		m_pieces[x] = new Palapeli::Piece*[yPieces];
		for (int y = 0; y < yPieces; ++y)
		{
			QPixmap pix(pieceWidth, pieceHeight);
			QPainter painter(&pix);
			painter.drawImage(QPoint(0, 0), image, QRect(x * pieceWidth, y * pieceHeight, pieceWidth, pieceHeight));
			painter.end();
			m_pieces[x][y] = new Palapeli::Piece(pix, this, x, y, pieceWidth, pieceHeight);
			Palapeli::Part* part = new Palapeli::Part(m_pieces[x][y], this);
			addItem(part);
			part->setPos(qrand() % (sceneWidth - pieceWidth), qrand() % (sceneHeight - pieceHeight));
			m_parts << part;
		}
	}
}

Palapeli::Piece* Palapeli::Scene::topNeighbor(int xIndex, int yIndex)
{
	return (yIndex == 0) ? 0 : m_pieces[xIndex][yIndex - 1]; 
}

Palapeli::Piece* Palapeli::Scene::bottomNeighbor(int xIndex, int yIndex)
{
	return (yIndex == m_yPieces - 1) ? 0 : m_pieces[xIndex][yIndex + 1]; 
}

Palapeli::Piece* Palapeli::Scene::leftNeighbor(int xIndex, int yIndex)
{
	return (xIndex == 0) ? 0 : m_pieces[xIndex - 1][yIndex]; 
}

Palapeli::Piece* Palapeli::Scene::rightNeighbor(int xIndex, int yIndex) 
{
	return (xIndex == m_xPieces - 1) ? 0 : m_pieces[xIndex + 1][yIndex]; 
}

void Palapeli::Scene::combineParts(Palapeli::Part* part1, Palapeli::Part* part2, qreal dx, qreal dy) //friend of Palapeli::Part
{
	QPointF pos1 = part1->pos();
	QPointF pos2 = part2->pos();
	dx += pos2.x() - pos1.x();
	dy += pos2.y() - pos1.y();
	if (!m_parts.contains(part2))
		return;
	foreach (Palapeli::Piece *piece, part2->m_pieces)
	{
		piece->setPart(part1);
		piece->moveBy(dx, dy);
	}
	m_parts.removeAll(part2);
	delete part2;
}

#include "scene.moc"

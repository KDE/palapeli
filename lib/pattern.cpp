/***************************************************************************
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

#include "pattern.h"

#include <ctime>
#include <QApplication>
#include <QImage>
#include <QList>

namespace Palapeli
{

	class PatternPrivate
	{
		public:
			PatternPrivate();
			~PatternPrivate();

			int m_pieceCounter;
			QSize m_imageSize;
			
			QList<QPointF> m_sceneBasePositions;
			bool m_loadSceneBasePositions;
			bool m_emittedAllPiecesGeneratedSignal;
	};

}

Palapeli::PatternPrivate::PatternPrivate()
	: m_pieceCounter(0)
	, m_loadSceneBasePositions(false)
	, m_emittedAllPiecesGeneratedSignal(false)
{
}

Palapeli::PatternPrivate::~PatternPrivate()
{
}

Palapeli::Pattern::Pattern()
	: p(new Palapeli::PatternPrivate)
{
}

Palapeli::Pattern::~Pattern()
{
	delete p;
}

void Palapeli::Pattern::loadPiecePositions(const QList<QPointF>& points)
{
	p->m_sceneBasePositions = points;
	p->m_loadSceneBasePositions = true;
}

void Palapeli::Pattern::slice(const QImage& image)
{
	p->m_imageSize = image.size();
	qsrand(time(0));
	doSlice(image);
}

void Palapeli::Pattern::reportPieceCount(int pieceCount)
{
	emit estimatePieceCountAvailable(pieceCount);
}

void Palapeli::Pattern::addPiece(const QImage& image, const QRectF& positionInImage)
{
	if (p->m_loadSceneBasePositions)
		emit pieceGenerated(image, positionInImage, p->m_sceneBasePositions.value(p->m_pieceCounter));
	else
	{
		const int sceneWidth = 2 * p->m_imageSize.width();
		const int sceneHeight = 2 * p->m_imageSize.height();
		const int leftEdgePos = qrand() % (sceneWidth - (int) positionInImage.width());
		const int topEdgePos = qrand() % (sceneHeight - (int) positionInImage.height());
		const QPointF basePosition(leftEdgePos - positionInImage.left(), topEdgePos - positionInImage.top());
		emit pieceGenerated(image, positionInImage, basePosition);
	}
	++p->m_pieceCounter;
}

void Palapeli::Pattern::addRelation(int piece1Id, int piece2Id)
{
	//at this point, we assume that all pieces have been created
	if (!p->m_emittedAllPiecesGeneratedSignal)
	{
		emit allPiecesGenerated();
		p->m_emittedAllPiecesGeneratedSignal = true;
	}
	emit relationGenerated(piece1Id, piece2Id);
}

#include "pattern.moc"

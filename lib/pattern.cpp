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
#include "pattern-executor.h"

#include <ctime>
#include <QApplication>
#include <QImage>
#include <QList>
#include <QPainter>

namespace Palapeli
{

	class PatternPrivate
	{
		public:
			PatternPrivate();
			~PatternPrivate();

			PatternExecutor* m_executor;

			int m_pieceCounter;
			QSize m_imageSize;
			qreal m_sceneSizeFactor;
			
			QList<QPointF> m_sceneBasePositions;
			bool m_loadSceneBasePositions;
			bool m_emittedAllPiecesGeneratedSignal;
	};

}

Palapeli::PatternPrivate::PatternPrivate()
	: m_executor(0)
	, m_pieceCounter(0)
	, m_sceneSizeFactor(2.0) //a reasonable default
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

void Palapeli::Pattern::setSceneSizeFactor(qreal factor)
{
	p->m_sceneSizeFactor = factor;
}

void Palapeli::Pattern::slice(const QImage& image)
{
	p->m_imageSize = image.size();
	qsrand(time(0));
	doSlice(image);
}

QPointF Palapeli::Pattern::generateNextBasePosition(const QRectF& positionInImage)
{
	if (p->m_loadSceneBasePositions)
		return p->m_sceneBasePositions.value(p->m_pieceCounter++);
	else
	{
		++p->m_pieceCounter; //unused in this case
		const QSize sceneSize = p->m_sceneSizeFactor * p->m_imageSize;
		const int leftEdgePos = qrand() % (sceneSize.width() - (int) positionInImage.width());
		const int topEdgePos = qrand() % (sceneSize.height() - (int) positionInImage.height());
		return QPointF(leftEdgePos - positionInImage.left(), topEdgePos - positionInImage.top());
	}
}

void Palapeli::Pattern::reportPieceCount(int pieceCount)
{
	emit estimatePieceCountAvailable(pieceCount);
}

void Palapeli::Pattern::addPiece(const QImage& image, const QRectF& positionInImage)
{
	emit pieceGenerated(image, positionInImage, generateNextBasePosition(positionInImage));
}

void Palapeli::Pattern::addPiece(const QImage& baseImage, const QImage& mask, const QRectF& positionInImage)
{
	emit pieceGenerated(baseImage, mask, positionInImage, generateNextBasePosition(positionInImage));
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

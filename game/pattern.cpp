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
#include "manager.h"
#include "piece.h"
#include "piecerelation.h"

#include <QImage>
#include <QPixmap>

namespace Palapeli
{

	class PatternPrivate
	{
		public:
			PatternPrivate();
			~PatternPrivate();

			QList<Piece*> m_pieces;
	};

}

Palapeli::PatternPrivate::PatternPrivate()
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

void Palapeli::Pattern::addPiece(const QPixmap& pixmap, const QRectF& positionInImage)
{
	Palapeli::Piece* piece = new Palapeli::Piece(pixmap, positionInImage);
	ppMgr()->addPiece(piece);
	p->m_pieces << piece;
}

void Palapeli::Pattern::addRelation(int piece1Id, int piece2Id, const QPointF& positionDifference)
{
	ppMgr()->addRelation(Palapeli::PieceRelation(p->m_pieces[piece1Id], p->m_pieces[piece2Id], positionDifference));
}

#include "pattern.moc"

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

#include "piecerelation.h"
#include "part.h"
#include "piece.h"

Palapeli::PieceRelation::PieceRelation(Palapeli::Piece* piece1, Palapeli::Piece* piece2)
	: m_piece1(piece1)
	, m_piece2(piece2)
{
}

Palapeli::Piece* Palapeli::PieceRelation::piece1() const
{
	return m_piece1;
}

Palapeli::Piece* Palapeli::PieceRelation::piece2() const
{
	return m_piece2;
}

bool Palapeli::PieceRelation::piecesInRightPosition() const
{
	static const qreal maxInaccuracyFactor = 0.1;
	const QSizeF maxInaccuracy = maxInaccuracyFactor * m_piece1->size();
	const QPointF positionDifference = m_piece2->part()->basePosition() - m_piece1->part()->basePosition();
	return qAbs(positionDifference.x()) <= maxInaccuracy.width() && qAbs(positionDifference.y()) <= maxInaccuracy.height();
}

void Palapeli::PieceRelation::combine()
{
	Palapeli::Part* part1 = piece1()->part();
	Palapeli::Part* part2 = piece2()->part();
	while (part2->pieceCount() > 0)
	{
		Palapeli::Piece* piece = part2->pieceAt(0);
		part2->removePiece(piece);
		part1->addPiece(piece);
	}
	ppMgr()->removePart(part2);
	part1->update(); //adapt positions of added pieces
	delete part2;

}

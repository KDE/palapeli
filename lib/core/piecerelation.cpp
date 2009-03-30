/***************************************************************************
 *   Copyright 2008 Felix Lemke <lemke.felix@ages-skripte.org>
 *   Copyright 2008-2009 Stefan Majewsky <majewsky@gmx.net>
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
#include "engine.h"
#include "part.h"
#include "piece.h"
#include "settings.h"

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

bool Palapeli::PieceRelation::operator==(const PieceRelation& relation) const
{
	return (m_piece1 == relation.m_piece1 && m_piece2 == relation.m_piece2) || (m_piece1 == relation.m_piece2 && m_piece2 == relation.m_piece1);
}

bool Palapeli::PieceRelation::piecesInRightPosition() const
{
	const qreal maxInaccuracyFactor = qreal(Settings::snappingPrecision()) / 100.0;
	const QSizeF maxInaccuracy = maxInaccuracyFactor * m_piece1->size();
	const QPointF positionDifference = m_piece2->part()->pos() - m_piece1->part()->pos();
	return qAbs(positionDifference.x()) <= maxInaccuracy.width() && qAbs(positionDifference.y()) <= maxInaccuracy.height();
}

void Palapeli::PieceRelation::combine() const
{
	Palapeli::Part* part1 = m_piece1->part();
	Palapeli::Part* part2 = m_piece2->part();
	if (part1 == part2)
		return;
	else if (part1->pieces().count() > part2->pieces().count())
		insert(part1, part2);
	else
		insert(part2, part1);
}

bool Palapeli::PieceRelation::combined() const
{
	return m_piece1->part() == m_piece2->part();
}

void Palapeli::PieceRelation::insert(Palapeli::Part* target, Palapeli::Part* source) const
{
	const QList<Palapeli::Piece*> pieces = source->pieces();
	foreach (Palapeli::Piece* piece, pieces)
	{
		source->removePiece(piece);
		target->addPiece(piece);
	}
	source->engine()->removePart(source);
	delete source;
}

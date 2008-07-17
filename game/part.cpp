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

#include "part.h"
#include "manager.h"
#include "piece.h"

int currentZValue = 0;

Palapeli::Part::Part(Palapeli::Piece* piece)
	: m_basePosition(piece->pos() - piece->positionInImage())
{
	addPiece(piece);
}

Palapeli::Part::~Part()
{
	foreach (Palapeli::Piece* piece, m_pieces)
		delete piece;
}

int Palapeli::Part::pieceCount() const
{
	return m_pieces.count();
}

QPointF Palapeli::Part::basePosition() const
{
	return m_basePosition;
}

void Palapeli::Part::setBasePosition(const QPointF& basePosition)
{
	if (m_basePosition == basePosition)
		return;
	m_basePosition = basePosition;
	update();
	ppMgr()->updateGraphics();
}

Palapeli::Piece* Palapeli::Part::pieceAt(int index) const
{
	return m_pieces[index];
}

void Palapeli::Part::addPiece(Palapeli::Piece* piece)
{
	if (!m_pieces.contains(piece))
		m_pieces << piece;
	piece->setPart(this);
}

void Palapeli::Part::removePiece(Palapeli::Piece* piece)
{
	if (m_pieces.contains(piece))
	{
		m_pieces.removeAll(piece);
		piece->setPart(0);
	}
}

void Palapeli::Part::move(const QPointF& positionDifference)
{
	if (positionDifference.isNull())
		return;
	m_basePosition += positionDifference;
	update();
	ppMgr()->updateGraphics();
}

void Palapeli::Part::update()
{
	//iterate Z value to make elements of this part appear on top
	++currentZValue;
	//move every piece to the right position
	foreach (Palapeli::Piece* piece, m_pieces)
	{
		piece->setPos(m_basePosition + piece->positionInImage());
		piece->setZValue(currentZValue);
	}
}

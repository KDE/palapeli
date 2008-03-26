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
#include "piece.h"
#include "scene.h"

Palapeli::Part::Part(Palapeli::Piece* piece, Scene* scene)
	: QObject()
	, m_scene(scene)
{
	addPiece(piece);
}

Palapeli::Part::~Part()
{
	//The pieces are destroyed by the scene.
}

void Palapeli::Part::moveAllBy(qreal dx, qreal dy)
{
	foreach (Palapeli::Piece* piece, m_pieces)
		piece->moveBy(dx, dy);
}

void Palapeli::Part::addPiece(Palapeli::Piece* piece)
{
	m_pieces << piece;
	piece->setPart(this);
}

void Palapeli::Part::searchConnections()
{
	static const qreal maxInaccuracyFactor = 0.1;
	foreach (Palapeli::Piece* piece, m_pieces)
	{
		const qreal xMaxInaccuracy = maxInaccuracyFactor * piece->width();
		const qreal yMaxInaccuracy = maxInaccuracyFactor * piece->height();
		const QPointF myPos = piece->pos();
		foreach (Palapeli::Piece::NeighborInfo ni, piece->m_neighbors)
		{
			if (this == ni.piece->part())
				continue;
			const QPointF posDiff = ni.piece->pos() - myPos;
			const qreal dx = posDiff.x() - ni.relativeXPos;
			const qreal dy = posDiff.y() - ni.relativeYPos;
			if (qAbs(dx) <= xMaxInaccuracy && qAbs(dy) <= yMaxInaccuracy)
				m_scene->combineParts(this, ni.piece->part(), -dx, -dy);
		}
	}
}

#include "part.moc"

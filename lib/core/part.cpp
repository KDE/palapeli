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

#include "part.h"
#include "engine.h"
#include "piece.h"
#include "view.h"

#include <QGraphicsSceneMouseEvent>

Palapeli::Part::Part(Palapeli::Piece* piece, Palapeli::Engine* engine)
	: m_engine(engine)
{
	addPiece(piece);
	setAcceptedMouseButtons(Qt::LeftButton);
// 	setHandlesChildEvents(true);
}

Palapeli::Part::~Part()
{
	qDeleteAll(m_pieces); //TODO: necessary?
}

QList<Palapeli::Piece*> Palapeli::Part::pieces() const
{
	return m_pieces;
}

Palapeli::Engine* Palapeli::Part::engine() const
{
	return m_engine;
}

void Palapeli::Part::addPiece(Palapeli::Piece* piece)
{
	if (!m_pieces.contains(piece))
		m_pieces << piece;
	piece->setPart(this);
	piece->setParentItem(this);
	piece->setPos(piece->positionInImage());
}

void Palapeli::Part::removePiece(Palapeli::Piece* piece)
{
	if (m_pieces.contains(piece))
	{
		m_pieces.removeAll(piece);
		piece->setPart(0);
		piece->setParentItem(0);
	}
}

void Palapeli::Part::setPosition(const QPointF& position)
{
	//check if pieces would go out of the scene because of this move
	QPointF newPos(position);
	foreach (Palapeli::Piece* piece, m_pieces)
	{
		piece->makePositionValid(newPos);
	}
	//move
	m_engine->view()->moveToTop(this);
	setPos(newPos);
}

#include <KDebug>

void Palapeli::Part::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	kDebug() << "works";
	const QPointF difference = event->scenePos() - event->lastScenePos();
	setPosition(pos() + difference);
}

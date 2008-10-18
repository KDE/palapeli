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

#include "engine.h"

#include "part.h"
#include "piece.h"
#include "piecerelation.h"
#include "view.h"

#include <QList>
#include <QPointer>

namespace Palapeli
{

	class EnginePrivate
	{
		public:
			EnginePrivate();

			QList<Part*> m_parts;
			QList<Piece*> m_pieces;
			QList<PieceRelation> m_relations;
			QPointer<View> m_view; //this object might get deleted by a KXmlGuiWindow using it
	};

}

Palapeli::EnginePrivate::EnginePrivate()
	: m_view(new Palapeli::View)
{
}

Palapeli::Engine* Palapeli::Engine::self()
{
	static Palapeli::Engine theOneAndOnly;
	return &theOneAndOnly;
}

Palapeli::Engine::Engine()
	: p(new Palapeli::EnginePrivate)
{
	connect(p->m_view, SIGNAL(viewportMoved()), this, SIGNAL(viewportMoved()));
	connect(p->m_view, SIGNAL(viewportScaled()), this, SIGNAL(viewportMoved()));
	connect(this, SIGNAL(pieceMoved()), this, SLOT(searchConnections()));
}

Palapeli::Engine::~Engine()
{
	//I see no benefit in declaring a separate destructor for Palapeli::EnginePrivate.
	foreach (Palapeli::Part* part, p->m_parts)
		delete part; //this does also delete the pieces
	if (!p->m_view.isNull())
		delete p->m_view;
	delete p;
}

int Palapeli::Engine::partCount() const
{
	return p->m_parts.count();
}

Palapeli::Part* Palapeli::Engine::partAt(int index) const
{
	return p->m_parts.value(index, 0);
}

int Palapeli::Engine::pieceCount() const
{
	return p->m_pieces.count();
}

Palapeli::Piece* Palapeli::Engine::pieceAt(int index) const
{
	return p->m_pieces.value(index, 0);
}

int Palapeli::Engine::relationCount() const
{
	return p->m_relations.count();
}

const Palapeli::PieceRelation& Palapeli::Engine::relationAt(int index) const
{
	return p->m_relations.at(index);
}

Palapeli::View* Palapeli::Engine::view() const
{
	return p->m_view;
}

void Palapeli::Engine::addPiece(Palapeli::Piece* piece, const QPointF& sceneBasePosition)
{
	p->m_pieces << piece;
	p->m_parts << new Palapeli::Part(piece);
	piece->part()->setBasePosition(sceneBasePosition);
	p->m_view->realScene()->addItem(piece);
}

void Palapeli::Engine::addRelation(int piece1Id, int piece2Id)
{
	Palapeli::PieceRelation relation(p->m_pieces[piece1Id], p->m_pieces[piece2Id]);
	if (!p->m_relations.contains(relation))
		p->m_relations << relation;
}

void Palapeli::Engine::removePart(Palapeli::Part* part)
{
	p->m_parts.removeAll(part);
}

void Palapeli::Engine::clear()
{
	foreach (Palapeli::Part* part, p->m_parts)
		delete part; //this does also delete the pieces
	p->m_parts.clear();
	p->m_pieces.clear();
	p->m_relations.clear();
}

void Palapeli::Engine::searchConnections()
{
	bool combinedSomething = false;
	foreach (const Palapeli::PieceRelation& rel, p->m_relations)
	{
		if (rel.piece1()->part() == rel.piece2()->part()) //already combined
			continue;
		if (rel.piecesInRightPosition())
		{
			rel.combine();
			combinedSomething = true;
		}
	}
	if (combinedSomething)
		emit relationsCombined();
}

#include "engine.moc"

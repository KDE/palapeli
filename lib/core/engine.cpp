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
#include "../textprogressbar.h"
#include "view.h"

#include <QList>
#include <QPointer>
#include <KLocalizedString>

namespace Palapeli
{

	class EnginePrivate
	{
		public:
			EnginePrivate();

			QList<Part*> m_parts;
			QList<Piece*> m_pieces;
			QList<PieceRelation> m_relations;
			TextProgressBar m_progress;
			QPointer<View> m_view;
			//in some cases, it is not safe to delete the view because of automatic deletion by the widget parent
			bool m_deleteViewInDestructor;
	};

}

Palapeli::EnginePrivate::EnginePrivate()
	: m_view(new Palapeli::View)
	, m_deleteViewInDestructor(true)
{
}

Palapeli::Engine::Engine()
	: p(new Palapeli::EnginePrivate)
{
	connect(p->m_view, SIGNAL(viewportMoved()), this, SIGNAL(viewportMoved()));
	connect(p->m_view, SIGNAL(viewportScaled()), this, SIGNAL(viewportMoved()));
	connect(this, SIGNAL(pieceMoved()), this, SLOT(searchConnections()));
}

void Palapeli::Engine::setDeleteViewInDestructor(bool deleteViewInDestructor)
{
	p->m_deleteViewInDestructor = deleteViewInDestructor;
}

Palapeli::Engine::~Engine()
{
	//I see no benefit in declaring a separate destructor for Palapeli::EnginePrivate.
	foreach (Palapeli::Part* part, p->m_parts)
		delete part; //this does also delete the pieces
	if (!p->m_view.isNull() && p->m_deleteViewInDestructor)
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

Palapeli::TextProgressBar* Palapeli::Engine::progressBar() const
{
	return &p->m_progress;
}

Palapeli::View* Palapeli::Engine::view() const
{
	return p->m_view;
}

void Palapeli::Engine::addPiece(Palapeli::Piece* piece, const QPointF& sceneBasePosition)
{
	p->m_pieces << piece;
	p->m_parts << new Palapeli::Part(piece, this);
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
	flushProgress();
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
	{
		updateProgress();
		emit relationsCombined();
	}
}

void Palapeli::Engine::updateProgress()
{
	const int partCount = p->m_parts.count(), pieceCount = p->m_pieces.count();
	if (p->m_progress.minimum() != 0)
		p->m_progress.setMinimum(0);
	if (p->m_progress.maximum() != pieceCount - 1)
		p->m_progress.setMaximum(pieceCount - 1);
	const int value = pieceCount - partCount;
	if (p->m_progress.value() != value)
	{
		p->m_progress.setValue(value);
		if (partCount == 1)
			p->m_progress.setText(i18n("You finished the puzzle."));
		else
		{
			int percentFinished = qreal(value) / qreal(pieceCount - 1) * 100;
			p->m_progress.setText(i18n("%1% finished", percentFinished));
		}
	}
}

void Palapeli::Engine::flushProgress()
{
	p->m_progress.setValue(p->m_progress.minimum());
	p->m_progress.setText(QString());
}

#include "engine.moc"

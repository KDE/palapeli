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

#ifndef PALAPELI_ENGINE_H
#define PALAPELI_ENGINE_H

#include <QObject>

namespace Palapeli
{

	class EnginePrivate;
	class Part;
	class Piece;
	class PieceRelation;

	class Engine : public QObject
	{
		Q_OBJECT
		public:
			static Engine* self();

			int partCount() const;
			Part* partAt(int index) const;
			int pieceCount() const;
			Piece* pieceAt(int index) const;
			int relationCount() const;
			PieceRelation relationAt(int index) const;
			View* view() const;
		public Q_SLOTS:
			void addPiece(Palapeli::Piece* piece, const QPointF& sceneBasePosition);
			void addRelation(int piece1Id, piece2Id);
			void removePart(Palapeli::Part* part);
			void clear();
		Q_SIGNALS:
			void viewportMoved();
			void piecePositionChanged();
			void pieceMoved();
		private:
			Engine();
			~Engine();
			Q_DISABLE_COPY(Engine)

			EnginePrivate* const p;
	};

}

//abbreviation for Palapeli::Engine::self()
inline Palapeli::Engine* ppEngine() { return Palapeli::Engine::self(); }

#endif // PALAPELI_ENGINE_H

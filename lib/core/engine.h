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

#include "../macros.h"
#include "part.h"
#include "piece.h"

#include <QObject>
#include <QPointF>

namespace Palapeli
{

	class EnginePrivate;
	class PieceRelation;
	class TextProgressBar;
	class View;

	class PALAPELIBASE_EXPORT Engine : public QObject
	{
		Q_OBJECT
		public:
			Engine();
			~Engine();

			int partCount() const;
			Part* partAt(int index) const;
			int pieceCount() const;
			Piece* pieceAt(int index) const;
			int relationCount() const;
			const PieceRelation& relationAt(int index) const;
			TextProgressBar* progressBar() const;
			View* view() const;
		public Q_SLOTS:
			void addPiece(Palapeli::Piece* piece, const QPointF& sceneBasePosition);
			void addRelation(int piece1Id, int piece2Id);
			void removePart(Palapeli::Part* part);
			void clear();

			void searchConnections();
			void updateProgress();
			void flushProgress();
		Q_SIGNALS:
			void piecePositionChanged();
			void pieceMoved();
			void relationsCombined();
			void viewportMoved();
		protected:
			friend class Part;
			friend class Piece;
		private:
			Q_DISABLE_COPY(Engine)

			EnginePrivate* const p;
	};

}

#endif // PALAPELI_ENGINE_H

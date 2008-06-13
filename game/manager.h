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

#ifndef PALAPELI_MANAGER_H
#define PALAPELI_MANAGER_H

#include <QObject>
#include <KUrl>

namespace Palapeli
{

	class MainWindow;
	class ManagerPrivate;
	class Minimap;
	class Part;
	class Pattern;
	class Piece;
	class PieceRelation;
	class Preview;
	class SavegameModel;
	class SavegameView;
	class View;

	class Manager : public QObject
	{
		Q_OBJECT
		public:
			Manager();
			~Manager();

			void addRelation(Piece* piece1, Piece* piece2, const QPointF& positionDifference);
			void searchConnections();
			void combine(Part* part1, Part* part2);
	
			Minimap* minimap() const;
			QListIterator<Part*> parts() const;
			Pattern* pattern() const;
			QListIterator<Piece*> pieces() const;
			Preview* preview() const;
			QListIterator<PieceRelation> relations() const;
			SavegameModel* savegameModel() const;
			SavegameView* savegameView() const;
			View* view() const;
			MainWindow* window() const;
		public Q_SLOTS:
			void updateMinimap();

			void createGame(const KUrl& url, int xPieceCount, int yPieceCount);
			void loadGame(const QString& name);
			bool saveGame(const QString& name);
			void deleteGame(const QString& name);
			void savegameWasCreated(const QString& name);
			void savegameWasDeleted(const QString& name);
		Q_SIGNALS:
			void gameLoaded(const QString& name);
			void savegameCreated(const QString& name);
			void savegameDeleted(const QString& name);
		private:
			ManagerPrivate* p;
	};

}

#endif //PALAPELI_MANAGER_H

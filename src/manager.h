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

#include <QImage>
#include <QObject>
#include <KUrl>

namespace Palapeli
{

	class MainWindow;
	class Minimap;
	class Part;
	class Pattern;
	class Piece;
	class PieceRelation;
	class Preview;
	class View;

	class Manager : public QObject
	{
		Q_OBJECT
		public:
			Manager();
			~Manager();

			QList<QString> availableSaveGames();
	
			void addRelation(Piece* piece1, Piece* piece2, const QPointF& positionDifference);
			void searchConnections();
			void combine(Part* part1, Part* part2);
	
			Minimap* minimap() const;
			QListIterator<Part*> parts() const;
			Pattern* pattern() const;
			QListIterator<Piece*> pieces() const;
			Preview* preview() const;
			QListIterator<PieceRelation> relations() const;
			View* view() const;
			MainWindow* window() const;
		public Q_SLOTS:
			void updateMinimap();

			void createGame(const KUrl& url, int xPieceCount, int yPieceCount);
			void loadGame(const QString& name);
			void saveGame(const QString& name);
			void deleteGame(const QString& name);
		Q_SIGNALS:
			void saveGameListUpdated();
			void gameLoaded(const QString& name);
		private:
			QString toLocalFile(const KUrl& url);
			void cleanupTempFiles();

			QImage m_image;
			QList<QString> m_localFiles;
			Minimap* m_minimap;
			QList<Part*> m_parts;
			Pattern* m_pattern;
			QList<Piece*> m_pieces;
			Preview* m_preview;
			QList<PieceRelation> m_relations;
			View* m_view;
			MainWindow* m_window;
	};

}

#endif //PALAPELI_MANAGER_H

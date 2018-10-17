/***************************************************************************
 *   Copyright 2009-2011 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef PALAPELI_COLLECTION_H
#define PALAPELI_COLLECTION_H

#include <QStandardItemModel>
#include <QMutex>

class KConfig;
class KConfigGroup;

namespace Palapeli
{
	class Puzzle;

	class Collection : public QStandardItemModel
	{
		Q_OBJECT
		public:
			enum Roles {
				NameRole = Qt::DisplayRole,
				ThumbnailRole = Qt::DecorationRole,
				CommentRole = Qt::UserRole + 1,
				AuthorRole,
				PieceCountRole,
				IsDeleteableRole,
				IdentifierRole
			};

			static Palapeli::Collection* instance();
			Palapeli::Puzzle* puzzleFromIndex(const QModelIndex& index) const;

			void importPuzzle(Palapeli::Puzzle* puzzle); ///< without copying!
			Palapeli::Puzzle* importPuzzle(const QString& path);
			void exportPuzzle(const QModelIndex& index, const QString& path);
			bool deletePuzzle(const QModelIndex& index);
		protected:
			Collection();
			virtual ~Collection();
		private:
			class Item;

			KConfig* m_config;
			KConfigGroup* m_group;
			QMutex m_configMutex;
	};
}

#endif // PALAPELI_COLLECTION_H

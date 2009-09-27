/***************************************************************************
 *   Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef PALAPELI_LIBRARYMODEL_H
#define PALAPELI_LIBRARYMODEL_H

#include <QAbstractListModel>

namespace Palapeli
{
	class PuzzleReader;

	class LibraryModel : public QAbstractListModel
	{
		Q_OBJECT
		public:
			enum Roles {
				IdentifierRole = Qt::UserRole + 1,
				NameRole,
				CommentRole,
				AuthorRole,
				ThumbnailRole
			};

			LibraryModel();
			virtual ~LibraryModel();

			virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
			virtual QVariant data(const QModelIndex& index, int role) const;
			virtual Qt::ItemFlags flags(const QModelIndex& index) const;
			Palapeli::PuzzleReader* puzzle(const QModelIndex& index) const;
		private:
			QList<Palapeli::PuzzleReader*> m_puzzles;
	};
}

#endif // PALAPELI_LIBRARYMODEL_H

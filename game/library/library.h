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

#ifndef PALAPELI_LIBRARY_H
#define PALAPELI_LIBRARY_H

#include <QAbstractListModel>

namespace ThreadWeaver
{
	class Job;
	class Weaver;
}

namespace Palapeli
{

	class LibraryBase;
	class PuzzleInfo;

	class Library : public QAbstractListModel
	{
		Q_OBJECT
		public:
			enum UserRoles
			{
				IdentifierRole = Qt::UserRole + 1,
				CommentRole,
				AuthorRole,
				PieceCountRole,
				ImageRole,
				PatternIconNameRole
			};

			static const int IconSize = 64;

			Library(Palapeli::LibraryBase* base);
			virtual ~Library();

			virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
			virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
			Palapeli::LibraryBase* base() const;
			Palapeli::PuzzleInfo* infoForPuzzle(const QModelIndex& index) const;
			Palapeli::PuzzleInfo* infoForPuzzle(int index) const;
			Palapeli::PuzzleInfo* infoForPuzzle(const QString& identifier) const;
		private Q_SLOTS:
			void loaderFinished(ThreadWeaver::Job* job);
			void entryInserted(const QString& identifier);
			void entryRemoved(const QString& identifier);
		private:
			Palapeli::LibraryBase* m_base;
			QList<Palapeli::PuzzleInfo*> m_puzzles;
			ThreadWeaver::Weaver* m_weaver;
	};

}

#endif // PALAPELI_LIBRARY_H
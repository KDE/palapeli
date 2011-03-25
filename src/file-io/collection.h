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

#ifndef PALAPELI_COLLECTION_H
#define PALAPELI_COLLECTION_H

#include <QAbstractListModel>

namespace Palapeli
{
	class OldPuzzle;

	class Collection : public QAbstractListModel
	{
		Q_OBJECT //allow qobject_casts
		public:
			enum Roles {
				//invisible metadata
				IdentifierRole = Qt::UserRole + 1,
				IsDeleteableRole,
				//object references
				PuzzleObjectRole = Qt::UserRole + 11, //contains a QObject* which can be casted to Palapeli::OldPuzzle*
				//visible metadata
				NameRole = Qt::UserRole + 21,
				CommentRole,
				AuthorRole,
				PieceCountRole,
				ThumbnailRole
			};

			virtual ~Collection();

			QString name() const;
			virtual bool canImportPuzzles() const;
			virtual QModelIndex importPuzzle(const Palapeli::OldPuzzle* const puzzle);
			virtual bool canDeletePuzzle(const QModelIndex& index) const;
			virtual bool deletePuzzle(const QModelIndex& index);

			virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
			virtual QVariant data(const QModelIndex& index, int role) const;
		protected:
			Collection(); //"abstract base class"

			QModelIndex addPuzzle(Palapeli::OldPuzzle* puzzle, const QString& identifier = QString()); ///< If no identifier is given, one will be generated. Returns the model index of the new item.
			void removePuzzle(const QModelIndex& index);
			void setName(const QString& name);
		private:
			QString m_name;
			QList<QString> m_identifiers;
			QList<Palapeli::OldPuzzle*> m_puzzles;
	};
}

#endif // PALAPELI_COLLECTION_H

/*
    SPDX-FileCopyrightText: 2009-2011 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

			static Palapeli::Collection* instance(QWidget * = nullptr);
			Palapeli::Puzzle* puzzleFromIndex(const QModelIndex& index) const;

			void importPuzzle(Palapeli::Puzzle* puzzle); ///< without copying!
			Palapeli::Puzzle* importPuzzle(const QString& path);
			void exportPuzzle(const QModelIndex& index, const QString& path);
			bool deletePuzzle(const QModelIndex& index);
		protected:
			Collection(QWidget *);
			~Collection() override;
		private:
			class Item;

			KConfig* m_config;
			KConfigGroup* m_group;
	};
}

#endif // PALAPELI_COLLECTION_H

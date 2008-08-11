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

#ifndef PALAPELI_PUZZLELIBRARY_P_H
#define PALAPELI_PUZZLELIBRARY_P_H

class QAbstractItemDelegate;
#include <QAbstractListModel>
class QImage;
class QString;
#include <QThread>

namespace Palapeli
{

	struct PuzzleLibraryEntry
	{
		PuzzleLibraryEntry();
		PuzzleLibraryEntry(const QString& identifier, const QString& name, const QString& comment, const QString& author, const QImage& image, int pieceCount);
		QString identifier;
		QString name;
		QString comment;
		QString author;
		QImage image;
		int pieceCount;
	};

	class PuzzleLibraryEntryLoader : public QThread
	{
		Q_OBJECT
		protected:
			virtual void run();
		Q_SIGNALS:
			void itemGenerated(Palapeli::PuzzleLibraryEntry* entry);
		private:
			QList<PuzzleLibraryEntry*> m_generatedEntries;
	};

	class PuzzleLibraryModel : public QAbstractListModel
	{
		Q_OBJECT
		public:
			PuzzleLibraryModel(QObject* parent = 0);
			virtual ~PuzzleLibraryModel();
			virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
			virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
			void reload();

			enum
			{
				IdentifierRole = Qt::UserRole,
				CommentRole,
				AuthorRole,
				PieceCountRole
			};
		private Q_SLOTS:
			void itemGenerated(Palapeli::PuzzleLibraryEntry* entry);
		Q_SIGNALS:
			void flushed();
		private:
			QList<PuzzleLibraryEntry> m_entries;
			PuzzleLibraryEntryLoader m_loader;
	};

	class PuzzleLibraryDelegate : public QAbstractItemDelegate
	{
		public:
			PuzzleLibraryDelegate(QObject* parent = 0);
			virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
			void paintLine(const QString& text, QRect& boundingRect, QPainter* painter, const QStyleOptionViewItem& option) const;
			virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

			static const int IconSize = 64;
			static const int Margin = 5;
	};

	class PuzzleLibraryPrivate
	{
		public:
			PuzzleLibraryPrivate();

			PuzzleLibraryModel m_model;
			PuzzleLibraryDelegate m_delegate;
	};

}

#endif // PALAPELI_PUZZLELIBRARY_P_H

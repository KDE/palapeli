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

#ifndef PALAPELI_LIBRARYBASE_H
#define PALAPELI_LIBRARYBASE_H

#include <QObject>
#include <QStringList>
#include <QUuid>
class KStandardDirs;
class KTempDir;
#include <KUrl>

namespace Palapeli
{

	class Library;

	class LibraryBase : public QObject
	{
		Q_OBJECT
		public:
			enum FileType
			{
				MainConfigFile,
				StateConfigFile,
				ImageFile
			};
			virtual ~LibraryBase() {}
			virtual QString findFile(const QString& identifier, Palapeli::LibraryBase::FileType type, bool onlyLocal = false) const = 0;
			virtual QStringList findEntries() const = 0;
			virtual bool insertEntry(const QString& identifier, Palapeli::Library* sourceLibrary);
			bool insertEntries(Palapeli::Library* sourceLibrary); //imports everything from the given source
			virtual bool canRemoveEntry(const QString& identifier, Palapeli::Library* library);
			virtual bool removeEntry(const QString& identifier, Palapeli::Library* library);
		Q_SIGNALS:
			void entryInserted(const QString& identifier);
			void entryRemoved(const QString& identifier);
		public Q_SLOTS:
			void reportNewEntry(const QString& identifier);
	};

	//This base scans the puzzle library provided with Palapeli.
	class LibraryStandardBase : public LibraryBase
	{
		public:
			static LibraryStandardBase* self();

			virtual ~LibraryStandardBase();
			virtual QString findFile(const QString& identifier, Palapeli::LibraryBase::FileType type, bool onlyLocal = false) const;
			virtual QStringList findEntries() const;
		private:
			LibraryStandardBase();
			Q_DISABLE_COPY(LibraryStandardBase)

			KStandardDirs* m_dirs;
	};

	//This base reads a puzzle archive. If the archive is corrupted or does not exist, this base is initialized in an empty state, and can be filled with a puzzle from another library. An existing puzzle will be overwritten when inserting another puzzle.
	class LibraryArchiveBase : public LibraryBase
	{
		public:
			LibraryArchiveBase(const KUrl& url);
			virtual ~LibraryArchiveBase();

			virtual QString findFile(const QString& identifier, Palapeli::LibraryBase::FileType type, bool onlyLocal = false) const;
			virtual QStringList findEntries() const;
			virtual bool insertEntry(const QString& identifier, Palapeli::Library* sourceLibrary);
		private:
			Q_DISABLE_COPY(LibraryArchiveBase)

			KUrl m_url;
			KTempDir* m_cache;
			QUuid m_identifier;
	};

}

#endif // PALAPELI_LIBRARYBASE_H

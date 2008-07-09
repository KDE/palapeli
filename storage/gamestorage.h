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

#ifndef PALAPELI_GAMESTORAGE_H
#define PALAPELI_GAMESTORAGE_H

#include "gamestorageitem.h"

#include <KUrl>

namespace Palapeli
{

	class GameStorageAttributes;
	class GameStoragePrivate;

	class GameStorage
	{
		public:
			GameStorage();
			GameStorage(const QString& baseDirectory);
			~GameStorage();

			bool accessible() const;
			QString baseDirectory() const;

			GameStorageItem addItem(const QString& extension, int type);
			GameStorageItem addItem(const KUrl& source, int type);
			GameStorageItem item(const QUuid& id);
			GameStorageItems queryItems(const GameStorageAttributes& attributes);
			bool removeItem(const GameStorageItem& item);

			bool addDependency(const GameStorageItem& source, const GameStorageItem& target);
			bool hasDependency(const GameStorageItem& source, const GameStorageItem& target);
			bool removeDependency(const GameStorageItem& source, const GameStorageItem& target);

			GameStorageItems importItems(GameStorage* storage, bool uniqueMetaData = true, const GameStorageItems& items = Palapeli::GameStorageItems());
			GameStorageItems importItems(const KUrl& archive, bool uniqueMetaData = true);
			bool exportItems(const KUrl& archive, const GameStorageItems& items, bool uniqueMetaData = true);

		private:
			Q_DISABLE_COPY(GameStorage)
			GameStoragePrivate* d;

			//interface for GameStorageItem
			friend class GameStorageItem;
			bool itemExists(const QUuid& id) const;
			QString itemExtension(const QUuid& id) const;
			int itemType(const QUuid& id) const;
			QString itemFilePath(const QUuid& id) const;
			QString itemMetaData(const QUuid& id) const;
			bool itemSetMetaData(const QUuid& id, const QString& text) const;
	};

}

#endif // PALAPELI_GAMESTORAGE_H

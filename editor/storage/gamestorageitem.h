/***************************************************************************
 *   Copyright 2008 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef PALAPELI_GAMESTORAGEITEM_H
#define PALAPELI_GAMESTORAGEITEM_H

#include <QString>
#include <QUuid>

namespace Palapeli
{

	class GameStorage;
	class GameStorageItemPrivate;

	class GameStorageItem
	{
		public:
			enum Type
			{
				InvalidType = -1,
				Unspecified = 0,
				GlobalConfig = 1,
				GlobalResource = 2,
				SavedGame = 3,
				Image = 4,
				UserType = 100 //applications can define their own types; the type IDs should start at GameStorageItem::UserType + 1
			};

			GameStorageItem();
			GameStorageItem(const GameStorageItem& other);
			~GameStorageItem();

			GameStorageItem& operator=(const GameStorageItem& other);
			bool operator==(const GameStorageItem& other) const;
			bool operator!=(const GameStorageItem& other) const;

			GameStorage* container() const;
			bool exists() const;
			QString extension() const;
			QString filePath() const;
			QUuid id() const;
			bool isNull() const;
			QString metaData() const;
			int type() const;

			bool setMetaData(const QString& text);

			operator QUuid() const;
		private:
			GameStorageItemPrivate* const d;

			//interface for GameStorage
			friend class GameStorage;
			GameStorageItem(const QUuid& id, GameStorage* container);
	};

	typedef QList<GameStorageItem> GameStorageItems;

}

#endif // PALAPELI_GAMESTORAGEITEM_H

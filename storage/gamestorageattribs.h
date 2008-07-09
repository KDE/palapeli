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

#ifndef PALAPELI_GAMESTORAGEATTRIBS_H
#define PALAPELI_GAMESTORAGEATTRIBS_H

//TODO: Negate attributes.
#include <QList>

namespace Palapeli
{

	class GameStorageItem;
	class GameStorageTypeAttributePrivate;
	class GameStorageMetaAttributePrivate;
	class GameStorageDependencyAttributePrivate;
	class GameStorageNoDependencyAttributePrivate;
	class GameStorageAttributesPrivate;

	class GameStorageAttribute
	{
		public:
			virtual ~GameStorageAttribute();
			virtual bool test(const GameStorageItem& item) const = 0;
	};

	class GameStorageTypeAttribute : public GameStorageAttribute
	{
		public:
			GameStorageTypeAttribute(int type);
			virtual ~GameStorageTypeAttribute();
			virtual bool test(const GameStorageItem& item) const;
		private:
			GameStorageTypeAttributePrivate* d;
	};

	class GameStorageMetaAttribute : public GameStorageAttribute
	{
		public:
			GameStorageMetaAttribute(const QString& text);
			virtual ~GameStorageMetaAttribute();
			virtual bool test(const GameStorageItem& item) const;
		private:
			GameStorageMetaAttributePrivate* d;
	};

	class GameStorageDependencyAttribute : public GameStorageAttribute
	{
		public:
			enum Direction
			{
				SourceIsGiven = 1,
				TargetIsGiven = 2
			};

			GameStorageDependencyAttribute(const GameStorageItem& item, Direction direction);
			virtual ~GameStorageDependencyAttribute();
			virtual bool test(const GameStorageItem& item) const;
		private:
			GameStorageDependencyAttributePrivate* d;
	};

	class GameStorageNoDependencyAttribute : public GameStorageAttribute
	{
		public:
			GameStorageNoDependencyAttribute();
			virtual ~GameStorageNoDependencyAttribute();
			virtual bool test(const GameStorageItem& item) const;
		private:
			GameStorageNoDependencyAttributePrivate* d;
	};

	class GameStorageAttributes
	{
		public:
			GameStorageAttributes();
			~GameStorageAttributes();
			GameStorageAttributes& operator<<(GameStorageAttribute* attribute);
			bool test(const GameStorageItem& item) const;
		private:
			GameStorageAttributesPrivate* d;
			Q_DISABLE_COPY(GameStorageAttributes)
	};

}

#endif // PALAPELI_GAMESTORAGEATTRIBS_H

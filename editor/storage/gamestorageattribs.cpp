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

#include "gamestorageattribs.h"
#include "gamestorage.h"
#include "gamestorageitem.h"

namespace Palapeli
{

	class GameStorageTypeAttributePrivate
	{
		public:
			GameStorageTypeAttributePrivate(int type) : m_type(type) {}

			int m_type;
	};

	class GameStorageMetaAttributePrivate
	{
		public:
			GameStorageMetaAttributePrivate(const QString& text) : m_text(text) {}

			QString m_text;
	};

	class GameStorageExtensionAttributePrivate
	{
		public:
			GameStorageExtensionAttributePrivate(const QString& extension) : m_extension(extension) {}

			QString m_extension;
	};

	class GameStorageDependencyAttributePrivate
	{
		public:
			GameStorageDependencyAttributePrivate(const GameStorageItem& item, GameStorageDependencyAttribute::Direction direction) : m_direction(direction), m_item(item) {}

			GameStorageDependencyAttribute::Direction m_direction;
			const GameStorageItem& m_item;
	};

	class GameStorageNoDependencyAttributePrivate
	{
		public:
			GameStorageNoDependencyAttributePrivate() {}
	};

	class GameStorageAttributesPrivate
	{
		public:
			QList<GameStorageAttribute*> m_attributes;
	};

}

Palapeli::GameStorageAttribute::~GameStorageAttribute()
{
}

Palapeli::GameStorageTypeAttribute::GameStorageTypeAttribute(int type)
	: d(new Palapeli::GameStorageTypeAttributePrivate(type))
{
}

Palapeli::GameStorageTypeAttribute::~GameStorageTypeAttribute()
{
	delete d;
}

bool Palapeli::GameStorageTypeAttribute::test(const Palapeli::GameStorageItem& item) const
{
	return !item.isNull() && item.type() == d->m_type;
}

Palapeli::GameStorageMetaAttribute::GameStorageMetaAttribute(const QString& text)
	: d(new Palapeli::GameStorageMetaAttributePrivate(text))
{
}

Palapeli::GameStorageMetaAttribute::~GameStorageMetaAttribute()
{
	delete d;
}

bool Palapeli::GameStorageMetaAttribute::test(const Palapeli::GameStorageItem& item) const
{
	return !item.isNull() && item.metaData() == d->m_text;
}

Palapeli::GameStorageExtensionAttribute::GameStorageExtensionAttribute(const QString& extension)
	: d(new Palapeli::GameStorageExtensionAttributePrivate(extension))
{
}

Palapeli::GameStorageExtensionAttribute::~GameStorageExtensionAttribute()
{
	delete d;
}

bool Palapeli::GameStorageExtensionAttribute::test(const Palapeli::GameStorageItem& item) const
{
	return !item.isNull() && item.extension() == d->m_extension;
}

Palapeli::GameStorageDependencyAttribute::GameStorageDependencyAttribute(const Palapeli::GameStorageItem& item, Palapeli::GameStorageDependencyAttribute::Direction direction)
	: d(new Palapeli::GameStorageDependencyAttributePrivate(item, direction))
{
}

Palapeli::GameStorageDependencyAttribute::~GameStorageDependencyAttribute()
{
	delete d;
}

bool Palapeli::GameStorageDependencyAttribute::test(const Palapeli::GameStorageItem& item) const
{
	switch (d->m_direction)
	{
		case Palapeli::GameStorageDependencyAttribute::SourceIsGiven:
			return item.container()->hasDependency(d->m_item, item);
		default: //case Palapeli::GameStorageDependencyAttribute::TargetIsGiven:
			return item.container()->hasDependency(item, d->m_item);
	}
}

Palapeli::GameStorageNoDependencyAttribute::GameStorageNoDependencyAttribute()
	//: d(new Palapeli::GameStorageNoDependencyAttributePrivate)
	: d(0)
{
}

Palapeli::GameStorageNoDependencyAttribute::~GameStorageNoDependencyAttribute()
{
	delete d;
}

bool Palapeli::GameStorageNoDependencyAttribute::test(const Palapeli::GameStorageItem& item) const
{
	Palapeli::GameStorageAttributes attribs1;
	attribs1 << new Palapeli::GameStorageDependencyAttribute(item, Palapeli::GameStorageDependencyAttribute::SourceIsGiven);
	if (item.container()->queryItems(attribs1).count() != 0)
		return false;
	Palapeli::GameStorageAttributes attribs2;
	attribs2 << new Palapeli::GameStorageDependencyAttribute(item, Palapeli::GameStorageDependencyAttribute::TargetIsGiven);
	return item.container()->queryItems(attribs2).count() == 0;
	
}

Palapeli::GameStorageAttributes::GameStorageAttributes()
	: d(new Palapeli::GameStorageAttributesPrivate)
{
}

Palapeli::GameStorageAttributes::~GameStorageAttributes()
{
	foreach(Palapeli::GameStorageAttribute* attribute, d->m_attributes)
		delete attribute;
	delete d;
}

Palapeli::GameStorageAttributes& Palapeli::GameStorageAttributes::operator<<(Palapeli::GameStorageAttribute* attribute)
{
	d->m_attributes << attribute;
	return *this;
}

bool Palapeli::GameStorageAttributes::test(const Palapeli::GameStorageItem& item) const
{
	foreach (Palapeli::GameStorageAttribute* attribute, d->m_attributes)
	{
		if (!attribute->test(item))
			return false;
	}
	return true;
}

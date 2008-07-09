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

#include "gamestorageitem.h"
#include "gamestorage.h"

namespace Palapeli
{

	class GameStorageItemPrivate
	{
		public:
			GameStorageItemPrivate(QUuid id, Palapeli::GameStorage* container);

			QUuid m_id;
			Palapeli::GameStorage* m_container;
	};

}

Palapeli::GameStorageItemPrivate::GameStorageItemPrivate(QUuid id, Palapeli::GameStorage* container)
	: m_id(id)
	, m_container(container)
{
}

Palapeli::GameStorageItem::GameStorageItem()
	: d(new Palapeli::GameStorageItemPrivate(QUuid(), 0))
{
}

Palapeli::GameStorageItem::GameStorageItem(const GameStorageItem& other)
	: d(new Palapeli::GameStorageItemPrivate(other.d->m_id, other.d->m_container))
{
}

Palapeli::GameStorageItem::GameStorageItem(const QUuid& id, Palapeli::GameStorage* container)
	: d(new Palapeli::GameStorageItemPrivate(id, container))
{
}

Palapeli::GameStorageItem::~GameStorageItem()
{
	delete d;
}

Palapeli::GameStorageItem& Palapeli::GameStorageItem::operator=(const Palapeli::GameStorageItem& other)
{
	d->m_id = other.d->m_id;
	d->m_container = other.d->m_container;
	return *this;
}

bool Palapeli::GameStorageItem::operator==(const Palapeli::GameStorageItem& other) const
{
	return d->m_id == other.d->m_id && d->m_container == other.d->m_container;
}

bool Palapeli::GameStorageItem::operator!=(const Palapeli::GameStorageItem& other) const
{
	return d->m_id != other.d->m_id || d->m_container != other.d->m_container;
}

Palapeli::GameStorage* Palapeli::GameStorageItem::container() const
{
	return d->m_container;
}

bool Palapeli::GameStorageItem::exists() const
{
	if (d->m_container == 0)
		return false;
	return d->m_container->itemExists(d->m_id);
}

QString Palapeli::GameStorageItem::extension() const
{
	if (d->m_container == 0)
		return QString();
	return d->m_container->itemExtension(d->m_id);
}

QString Palapeli::GameStorageItem::filePath() const
{
	if (d->m_container == 0)
		return QString();
	return d->m_container->itemFilePath(d->m_id);
}

QUuid Palapeli::GameStorageItem::id() const
{
	if (d->m_container == 0)
		return QUuid();
	return d->m_id;
}

bool Palapeli::GameStorageItem::isNull() const
{
	return d->m_id.isNull() || d->m_container == 0;
}

QString Palapeli::GameStorageItem::metaData() const
{
	if (d->m_container == 0)
		return QString();
	return d->m_container->itemMetaData(d->m_id);
}

int Palapeli::GameStorageItem::type() const
{
	if (d->m_container == 0)
		return Palapeli::GameStorageItem::InvalidType;
	return d->m_container->itemType(d->m_id);
}

bool Palapeli::GameStorageItem::setMetaData(const QString& text)
{
	if (d->m_container == 0)
		return false;
	return d->m_container->itemSetMetaData(d->m_id, text);
}

Palapeli::GameStorageItem::operator QUuid() const
{
	return d->m_id;
}

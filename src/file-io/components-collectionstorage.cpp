/***************************************************************************
 *   Copyright 2009-2011 Stefan Majewsky <majewsky@gmx.net>
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

#include "components.h"

#include <QBuffer>
#include <QFileInfo>
#include <KConfigGroup>
#include <QMutex>

Palapeli::CollectionStorageComponent::CollectionStorageComponent(KConfigGroup* group, QMutex *groupMutex)
	: m_group(group), m_groupMutex(groupMutex)
{
}

Palapeli::CollectionStorageComponent::~CollectionStorageComponent()
{
	delete m_group;
}

Palapeli::PuzzleComponent* Palapeli::CollectionStorageComponent::cast(Type type) const
{
	const QString file = puzzle()->location();
	//everything except for metadata must always be read from archive
	if (type == ArchiveStorage)
		return new Palapeli::ArchiveStorageComponent;
	else if (type != Metadata)
	{
		const Palapeli::PuzzleComponent* arStorage = puzzle()->get(ArchiveStorage);
		return arStorage ? arStorage->cast(type) : 0;
	}
	//try to serve metadata from cache
	const QDateTime mtime = QFileInfo(file).lastModified();
	m_groupMutex->lock();
	if (m_group->readEntry("ModifyDateTime", QString()) == mtime.toString())
	{
		//cache is up-to-date
		Palapeli::PuzzleMetadata metadata;
		metadata.name = m_group->readEntry("Name", QString());
		metadata.comment = m_group->readEntry("Comment", QString());
		metadata.author = m_group->readEntry("Author", QString());
		metadata.pieceCount = m_group->readEntry("PieceCount", 0);
		metadata.modifyProtection = m_group->readEntry("ModifyProtection", false);
		QByteArray ar = m_group->readEntry("Thumbnail", QByteArray());
		metadata.thumbnail.loadFromData(QByteArray::fromBase64 (ar));
		m_groupMutex->unlock();
		return new Palapeli::MetadataComponent(metadata);
	}
	else
	{
		m_groupMutex->unlock();
		//read metadata from archive...
		const Palapeli::PuzzleComponent* arStorage = puzzle()->get(ArchiveStorage);
		if (!arStorage)
			return 0;
		Palapeli::PuzzleComponent* cMetadata = arStorage->cast(Metadata);
		if (!cMetadata)
			return 0;
		//...and populate cache (image is written via a buffer
		//because KConfig does not support QImage directly)
		const Palapeli::PuzzleMetadata metadata = dynamic_cast<Palapeli::MetadataComponent*>(cMetadata)->metadata;
		QBuffer buffer;
		metadata.thumbnail.save(&buffer, "PNG");
		m_groupMutex->lock();
		m_group->writeEntry("Name", metadata.name);
		m_group->writeEntry("Comment", metadata.comment);
		m_group->writeEntry("Author", metadata.author);
		m_group->writeEntry("PieceCount", metadata.pieceCount);
		m_group->writeEntry("ModifyProtection", metadata.modifyProtection);
		m_group->writeEntry("ModifyDateTime", mtime.toString());
		m_group->writeEntry("Thumbnail", buffer.data().toBase64 ());
		m_group->sync();
		m_groupMutex->unlock();
		return cMetadata;
	}
}

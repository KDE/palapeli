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

#include <QtCore/QBuffer>
#include <QtCore/QFileInfo>
#include <KDE/KConfigGroup>

Palapeli::CollectionStorageComponent::CollectionStorageComponent(KConfigGroup* group)
	: m_group(group)
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
	if (m_group->readEntry("ModifyDateTime", QDateTime()) == mtime)
	{
		//cache is up-to-date
		Palapeli::PuzzleMetadata metadata;
		metadata.name = m_group->readEntry("Name", QString());
		metadata.comment = m_group->readEntry("Comment", QString());
		metadata.author = m_group->readEntry("Author", QString());
		metadata.pieceCount = m_group->readEntry("PieceCount", 0);
		metadata.modifyProtection = m_group->readEntry("ModifyProtection", false);
		metadata.thumbnail.loadFromData(m_group->readEntry("Thumbnail", QByteArray()));
		return new Palapeli::MetadataComponent(metadata);
	}
	else
	{
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
		m_group->writeEntry("Name", metadata.name);
		m_group->writeEntry("Comment", metadata.comment);
		m_group->writeEntry("Author", metadata.author);
		m_group->writeEntry("PieceCount", metadata.pieceCount);
		m_group->writeEntry("ModifyProtection", metadata.modifyProtection);
		m_group->writeEntry("ModifyDateTime", mtime);
		QBuffer buffer;
		metadata.thumbnail.save(&buffer, "PNG");
		m_group->writeEntry("Thumbnail", buffer.data());
		m_group->sync();
		return cMetadata;
	}
}

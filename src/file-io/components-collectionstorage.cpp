/*
    SPDX-FileCopyrightText: 2009-2011 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "components.h"

#include <QBuffer>
#include <QFileInfo>
#include <KConfigGroup>

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
		return arStorage ? arStorage->cast(type) : nullptr;
	}
	//try to serve metadata from cache
	const QDateTime mtime = QFileInfo(file).lastModified();
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
		return new Palapeli::MetadataComponent(metadata);
	}
	else
	{
		//read metadata from archive...
		const Palapeli::PuzzleComponent* arStorage = puzzle()->get(ArchiveStorage);
		if (!arStorage)
			return nullptr;
		Palapeli::PuzzleComponent* cMetadata = arStorage->cast(Metadata);
		if (!cMetadata)
			return nullptr;
		//...and populate cache (image is written via a buffer
		//because KConfig does not support QImage directly)
		const Palapeli::PuzzleMetadata metadata = dynamic_cast<Palapeli::MetadataComponent*>(cMetadata)->metadata;
		QBuffer buffer;
		metadata.thumbnail.save(&buffer, "PNG");
		m_group->writeEntry("Name", metadata.name);
		m_group->writeEntry("Comment", metadata.comment);
		m_group->writeEntry("Author", metadata.author);
		m_group->writeEntry("PieceCount", metadata.pieceCount);
		m_group->writeEntry("ModifyProtection", metadata.modifyProtection);
		m_group->writeEntry("ModifyDateTime", mtime.toString());
		m_group->writeEntry("Thumbnail", buffer.data().toBase64 ());
		return cMetadata;
	}
}

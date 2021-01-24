/*
    SPDX-FileCopyrightText: 2011 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "components.h"

#include <QFile>

Palapeli::CopyComponent::CopyComponent(Palapeli::Puzzle* puzzle)
	: m_puzzle(puzzle)
{
}

Palapeli::PuzzleComponent* Palapeli::CopyComponent::cast(Type type) const
{
	//get component from other puzzle
	if (type == Metadata)
	{
		const Palapeli::PuzzleComponent* c = m_puzzle->get(Metadata);
		const Palapeli::MetadataComponent* cmp = dynamic_cast<const Palapeli::MetadataComponent*>(c);
		return cmp ? new Palapeli::MetadataComponent(cmp->metadata) : nullptr;
	}
	else if (type == Contents)
	{
		const Palapeli::PuzzleComponent* c = m_puzzle->get(Contents);
		const Palapeli::ContentsComponent* cmp = dynamic_cast<const Palapeli::ContentsComponent*>(c);
		return cmp ? new Palapeli::ContentsComponent(cmp->contents) : nullptr;
	}
	else if (type == CreationContext)
	{
		const Palapeli::PuzzleComponent* c = m_puzzle->get(CreationContext);
		const Palapeli::CreationContextComponent* cmp = dynamic_cast<const Palapeli::CreationContextComponent*>(c);
		return cmp ? new Palapeli::CreationContextComponent(cmp->creationContext) : nullptr;
	}
	//casts for writing an archive
	else if (type == DirectoryStorage)
		return Palapeli::DirectoryStorageComponent::fromData(puzzle());
	else if (type == ArchiveStorage)
	{
		if (!m_puzzle->component(ArchiveStorage) && !m_puzzle->component(CollectionStorage))
			return Palapeli::ArchiveStorageComponent::fromData(puzzle());
		//optimization: if the other puzzle has an archive or collection
		//storage available, copy that instead of recreating everything
		m_puzzle->get(ArchiveStorage);
		QFile otherFile(m_puzzle->location());
		if (otherFile.copy(puzzle()->location()))
			return new Palapeli::ArchiveStorageComponent;
		else
			return nullptr;
	}
	//unknown type requested
	else
		return nullptr;
}

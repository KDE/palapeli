/***************************************************************************
 *   Copyright 2011 Stefan Majewsky <majewsky@gmx.net>
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

#include <QtCore/QFile>
#include <QtCore/QVariant>

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
		return cmp ? new Palapeli::MetadataComponent(cmp->metadata) : 0;
	}
	else if (type == Contents)
	{
		const Palapeli::PuzzleComponent* c = m_puzzle->get(Contents);
		const Palapeli::ContentsComponent* cmp = dynamic_cast<const Palapeli::ContentsComponent*>(c);
		return cmp ? new Palapeli::ContentsComponent(cmp->contents) : 0;
	}
	else if (type == CreationContext)
	{
		const Palapeli::PuzzleComponent* c = m_puzzle->get(CreationContext);
		const Palapeli::CreationContextComponent* cmp = dynamic_cast<const Palapeli::CreationContextComponent*>(c);
		return cmp ? new Palapeli::CreationContextComponent(cmp->creationContext) : 0;
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
		m_puzzle->get(ArchiveStorage).waitForFinished();
		QFile otherFile(m_puzzle->location());
		if (otherFile.copy(puzzle()->location()))
			return new Palapeli::ArchiveStorageComponent;
		else
			return 0;
	}
	//unknown type requested
	else
		return 0;
}

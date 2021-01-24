/*
    SPDX-FileCopyrightText: 2009-2011 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "components.h"

#include <KTar>

Palapeli::ArchiveStorageComponent::ArchiveStorageComponent()
{
}

Palapeli::PuzzleComponent* Palapeli::ArchiveStorageComponent::cast(Palapeli::PuzzleComponent::Type type) const
{
	//any casting is done by first casting to DirectoryStorage
	if (type == DirectoryStorage)
	{
		//make archive available locally
		//open archive and extract into temporary directory
		KTar tar(puzzle()->location(), QStringLiteral("application/x-gzip"));
		if (!tar.open(QIODevice::ReadOnly))
			return nullptr;
		Palapeli::DirectoryStorageComponent* storage = new Palapeli::DirectoryStorageComponent;
		tar.directory()->copyTo(storage->directory());
		//cleanup
		tar.close();
		return storage;
	}
	else
	{
		const Palapeli::PuzzleComponent* dirStorage = puzzle()->get(DirectoryStorage);
		return dirStorage ? dirStorage->cast(type) : nullptr;
	}
}

Palapeli::ArchiveStorageComponent* Palapeli::ArchiveStorageComponent::fromData(Palapeli::Puzzle* puzzle)
{
	puzzle->get(DirectoryStorage);
	const Palapeli::DirectoryStorageComponent* dirStorage = puzzle->component<Palapeli::DirectoryStorageComponent>();
	//compress archive to location
	KTar tar(puzzle->location(), QStringLiteral("application/x-gzip"));
	if (!tar.open(QIODevice::WriteOnly))
		return nullptr;
	if (!tar.addLocalDirectory(dirStorage->directory(), QStringLiteral(".")))
		return nullptr;
	if (!tar.close())
		return nullptr;
	//done
	return new Palapeli::ArchiveStorageComponent;
}

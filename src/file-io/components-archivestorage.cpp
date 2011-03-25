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

#include <QtCore/QVariant>
#include <KDE/KTar>

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
		KTar tar(puzzle()->location(), "application/x-gzip");
		if (!tar.open(QIODevice::ReadOnly))
			return 0;
		Palapeli::DirectoryStorageComponent* storage = new Palapeli::DirectoryStorageComponent;
		tar.directory()->copyTo(storage->directory());
		//cleanup
		tar.close();
		return storage;
	}
	else
	{
		const Palapeli::PuzzleComponent* dirStorage = puzzle()->get(DirectoryStorage);
		return dirStorage ? dirStorage->cast(type) : 0;
	}
}

Palapeli::ArchiveStorageComponent* Palapeli::ArchiveStorageComponent::fromData(Palapeli::Puzzle* puzzle)
{
	puzzle->get(DirectoryStorage).waitForFinished();
	const Palapeli::DirectoryStorageComponent* dirStorage = puzzle->component<Palapeli::DirectoryStorageComponent>();
	//compress archive to location
	KTar tar(puzzle->location(), "application/x-gzip");
	if (!tar.open(QIODevice::WriteOnly))
		return 0;
	if (!tar.addLocalDirectory(dirStorage->directory(), QLatin1String(".")))
		return 0;
	if (!tar.close())
		return 0;
	//done
	return new Palapeli::ArchiveStorageComponent;
}

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

#ifndef PALAPELI_COMPONENTS_H
#define PALAPELI_COMPONENTS_H

#include "puzzle.h"
#include "puzzlestructs.h"

class KTempDir;
#include <KDE/KUrl>

#define COMPONENT_SUBCLASS(mytype) \
	public: \
	enum { ComponentType = mytype }; \
	virtual Type type() const { return mytype; }

namespace Palapeli
{
	class MetadataComponent : public Palapeli::PuzzleComponent
	{
		COMPONENT_SUBCLASS(Metadata)
		public:
			MetadataComponent(const Palapeli::PuzzleMetadata& metadata) : metadata(metadata) {}

			Palapeli::PuzzleMetadata metadata;
	};

	class ContentsComponent : public Palapeli::PuzzleComponent
	{
		COMPONENT_SUBCLASS(Contents)
		public:
			ContentsComponent(const Palapeli::PuzzleContents& contents) : contents(contents) {}

			Palapeli::PuzzleContents contents;
	};

	class CreationContextComponent : public Palapeli::PuzzleComponent
	{
		COMPONENT_SUBCLASS(CreationContext)
		public:
			CreationContextComponent(const Palapeli::PuzzleCreationContext& creationContext) : creationContext(creationContext) {}

			Palapeli::PuzzleCreationContext creationContext;
			virtual Palapeli::PuzzleComponent* cast(Type type) const;
	};

	class DirectoryStorageComponent : public Palapeli::PuzzleComponent
	{
		COMPONENT_SUBCLASS(DirectoryStorage)
		public:
			DirectoryStorageComponent();
			virtual ~DirectoryStorageComponent();

			QString directory() const;
			virtual Palapeli::PuzzleComponent* cast(Type type) const;
		private:
			KTempDir* m_dir;
	};

	class ArchiveStorageComponent : public Palapeli::PuzzleComponent
	{
		COMPONENT_SUBCLASS(ArchiveStorage)
		public:
			ArchiveStorageComponent(const KUrl& location);

			virtual Palapeli::PuzzleComponent* cast(Type type) const;
		private:
			KUrl m_location;
	};
}

#undef COMPONENT_SUBCLASS

#endif // PALAPELI_COMPONENTS_H

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

class KConfigGroup;
class QTemporaryDir;

#define COMPONENT_SUBCLASS(mytype) \
	public: \
	enum { ComponentType = mytype }; \
	Type type() const Q_DECL_OVERRIDE { return mytype; }

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

	///This is a valid mainComponent.
	class CreationContextComponent : public Palapeli::PuzzleComponent
	{
		COMPONENT_SUBCLASS(CreationContext)
		public:
			CreationContextComponent(const Palapeli::PuzzleCreationContext& creationContext) : creationContext(creationContext) {}

			Palapeli::PuzzleCreationContext creationContext;
			Palapeli::PuzzleComponent* cast(Type type) const Q_DECL_OVERRIDE;
	};

	///This component copies the data (i.e. everything in puzzlestructs.h) from
	///an existing puzzle. This is a valid mainComponent.
	class CopyComponent : public Palapeli::PuzzleComponent
	{
		COMPONENT_SUBCLASS(Copy)
		public:
			CopyComponent(Palapeli::Puzzle* puzzle);

			Palapeli::PuzzleComponent* cast(Type type) const Q_DECL_OVERRIDE;
		private:
			Palapeli::Puzzle* m_puzzle;
	};

	///This is a valid mainComponent.
	class DirectoryStorageComponent : public Palapeli::PuzzleComponent
	{
		COMPONENT_SUBCLASS(DirectoryStorage)
		public:
			DirectoryStorageComponent();
			static Palapeli::DirectoryStorageComponent* fromData(Palapeli::Puzzle* puzzle);
			virtual ~DirectoryStorageComponent();

			QString directory() const;
			Palapeli::PuzzleComponent* cast(Type type) const Q_DECL_OVERRIDE;
		private:
			QTemporaryDir* m_dir;
	};

	///This is a valid mainComponent.
	class ArchiveStorageComponent : public Palapeli::PuzzleComponent
	{
		COMPONENT_SUBCLASS(ArchiveStorage)
		public:
			ArchiveStorageComponent();
			static Palapeli::ArchiveStorageComponent* fromData(Palapeli::Puzzle* puzzle);

			Palapeli::PuzzleComponent* cast(Type type) const Q_DECL_OVERRIDE;
	};

	///This is a valid mainComponent.
	class CollectionStorageComponent : public Palapeli::PuzzleComponent
	{
		COMPONENT_SUBCLASS(CollectionStorage)
		public:
			///Takes ownership of @a group.
			CollectionStorageComponent(KConfigGroup* group, QMutex *groupMutex);
			virtual ~CollectionStorageComponent();

			Palapeli::PuzzleComponent* cast(Type type) const Q_DECL_OVERRIDE;
		private:
			KConfigGroup* m_group;
			QMutex *m_groupMutex;
	};

	///This is used by the collection if, instead of an actual puzzle archive,
	///only a desktop file and an image is available (like for the puzzles from
	///the default collection).
	class RetailStorageComponent : public Palapeli::PuzzleComponent
	{
		COMPONENT_SUBCLASS(RetailStorage)
		public:
			RetailStorageComponent(const QString& desktopFile);

			Palapeli::PuzzleComponent* cast(Type type) const Q_DECL_OVERRIDE;
		private:
			QString m_desktopFile;
	};
}

#undef COMPONENT_SUBCLASS

#endif // PALAPELI_COMPONENTS_H

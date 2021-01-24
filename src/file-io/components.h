/*
    SPDX-FileCopyrightText: 2011 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELI_COMPONENTS_H
#define PALAPELI_COMPONENTS_H

#include "puzzle.h"
#include "puzzlestructs.h"

class KConfigGroup;
class QTemporaryDir;
class QMutex;

#define COMPONENT_SUBCLASS(mytype) \
	public: \
	enum { ComponentType = mytype }; \
	Type type() const override { return mytype; }

namespace Palapeli
{
	class MetadataComponent : public Palapeli::PuzzleComponent
	{
		COMPONENT_SUBCLASS(Metadata)
		public:
			explicit MetadataComponent(const Palapeli::PuzzleMetadata& metadata) : metadata(metadata) {}

			Palapeli::PuzzleMetadata metadata;
	};

	class ContentsComponent : public Palapeli::PuzzleComponent
	{
		COMPONENT_SUBCLASS(Contents)
		public:
			explicit ContentsComponent(const Palapeli::PuzzleContents& contents) : contents(contents) {}

			Palapeli::PuzzleContents contents;
	};

	///This is a valid mainComponent.
	class CreationContextComponent : public Palapeli::PuzzleComponent
	{
		COMPONENT_SUBCLASS(CreationContext)
		public:
			explicit CreationContextComponent(const Palapeli::PuzzleCreationContext& creationContext) : creationContext(creationContext) {}

			Palapeli::PuzzleCreationContext creationContext;
			Palapeli::PuzzleComponent* cast(Type type) const override;
	};

	///This component copies the data (i.e. everything in puzzlestructs.h) from
	///an existing puzzle. This is a valid mainComponent.
	class CopyComponent : public Palapeli::PuzzleComponent
	{
		COMPONENT_SUBCLASS(Copy)
		public:
			explicit CopyComponent(Palapeli::Puzzle* puzzle);

			Palapeli::PuzzleComponent* cast(Type type) const override;
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
			~DirectoryStorageComponent() override;

			QString directory() const;
			Palapeli::PuzzleComponent* cast(Type type) const override;
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

			Palapeli::PuzzleComponent* cast(Type type) const override;
	};

	///This is a valid mainComponent.
	class CollectionStorageComponent : public Palapeli::PuzzleComponent
	{
		COMPONENT_SUBCLASS(CollectionStorage)
		public:
			///Takes ownership of @a group.
			CollectionStorageComponent(KConfigGroup* group);
			~CollectionStorageComponent() override;

			Palapeli::PuzzleComponent* cast(Type type) const override;
		private:
			KConfigGroup* m_group;
	};

	///This is used by the collection if, instead of an actual puzzle archive,
	///only a desktop file and an image is available (like for the puzzles from
	///the default collection).
	class RetailStorageComponent : public Palapeli::PuzzleComponent
	{
		COMPONENT_SUBCLASS(RetailStorage)
		public:
			explicit RetailStorageComponent(const QString& desktopFile);

			Palapeli::PuzzleComponent* cast(Type type) const override;
		private:
			QString m_desktopFile;
	};
}

#undef COMPONENT_SUBCLASS

#endif // PALAPELI_COMPONENTS_H

/*
    SPDX-FileCopyrightText: 2011 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELI_PUZZLE_H
#define PALAPELI_PUZZLE_H

#include <QObject>
#include <QMetaType>

#include <iostream>
#define CAST_ERROR(x) std::cerr << qPrintable(x) << std::endl; //TODO: these errors should be reported to Puzzle

namespace Palapeli
{
	class Puzzle;

	class PuzzleComponent
	{
		public:
			enum Type
			{
				//data components
				Metadata = 0,        ///< Palapeli::MetadataComponent
				Contents,            ///< Palapeli::ContentsComponent
				CreationContext,     ///< Palapeli::CreationContextComponent
				//storage components
				Copy,                ///< Palapeli::CopyComponent
				DirectoryStorage,    ///< Palapeli::DirectoryStorageComponent
				ArchiveStorage,      ///< Palapeli::ArchiveStorageComponent
				CollectionStorage,   ///< Palapeli::CollectionStorageComponent
				RetailStorage        ///< Palapeli::RetailStorageComponent
			};

			PuzzleComponent();
			virtual ~PuzzleComponent();

			virtual Type type() const = 0;
			Palapeli::Puzzle* puzzle() const;
			///This method will be called in worker threads to create a new
			///component of the given @a type from the information in this
			///component. 0 is a useful result if the cast is not possible
			///for semantical reasons (e.g. cannot infer contents from
			///metadata) or technical reasons (e.g. puzzle file corrupted).
			///
			///The default implementation represents a non-castable
			///component and returns 0 for any type.
			virtual Palapeli::PuzzleComponent* cast(Type type) const;
		private:
			friend class Palapeli::Puzzle;
			Palapeli::Puzzle* m_puzzle;
	};

	class Puzzle : public QObject
	{
		Q_OBJECT
		public:
			///Takes ownership of @a mainComponent.
			Puzzle(Palapeli::PuzzleComponent* mainComponent, const QString& location, const QString& identifier);
			~Puzzle() override;

			///Returns an identifier for use with puzzles loaded from the file
			///system. Rationale: The identifier must be unique during the
			///session, but should also be the same for the same puzzle over
			///the course of multiple sessions (in order to store savegames).
			static QString fsIdentifier(const QString& location);

			///Returns the component for the given @a type, or 0 if this
			///component is not available. Access to the component storage is
			///not memory-ordered or anything. If you need a thread-safe
			///interface, use get() instead.
			const Palapeli::PuzzleComponent* component(Palapeli::PuzzleComponent::Type type) const;
			///@overload plus casting; same restrictions apply
			template <typename T> const T* component() const
			{
				return dynamic_cast<const T*>(component((Palapeli::PuzzleComponent::Type) T::ComponentType));
			}
			///Requests that the given component @a type is made available.
			const Palapeli::PuzzleComponent *get(Palapeli::PuzzleComponent::Type type);

			QString identifier() const;
			QString location() const;
			void setLocation(const QString& location);

			///Deletes the component with the given @a type from this puzzle.
			///Because the component might be in use elsewhere in the program,
			///this operation is highly dangerous, but it's nonetheless required
			///for the ContentsComponent because that one keeps very much data
			///in memory.
			void dropComponent(Palapeli::PuzzleComponent::Type type);
		private:
			struct Private;
			Private* const d;
	};
}

Q_DECLARE_METATYPE(Palapeli::Puzzle*)

#endif // PALAPELI_PUZZLE_H

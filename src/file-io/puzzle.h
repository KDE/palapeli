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

#ifndef PALAPELI_PUZZLE_H
#define PALAPELI_PUZZLE_H

#include <QtCore/QFuture>
#include <QtCore/QMetaType>

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
				DirectoryStorage,    ///< Palapeli::DirectoryStorageComponent
				ArchiveStorage       ///< Palapeli::ArchiveStorageComponent
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

	typedef QFuture<const Palapeli::PuzzleComponent*> Future;
	typedef QFutureWatcher<const Palapeli::PuzzleComponent*> FutureWatcher;

	class Puzzle : public QObject
	{
		Q_OBJECT
		public:
			///Takes ownership of @a mainComponent.
			Puzzle(Palapeli::PuzzleComponent* mainComponent);
			virtual ~Puzzle();

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
			///Use the returned future to get the component once the operation
			///has finished. The future's waitForFinished() method can be used
			///for synchronized access.
			Palapeli::Future get(Palapeli::PuzzleComponent::Type type);
		private:
			class Private;
			Private* const d;
	};
}

Q_DECLARE_METATYPE(Palapeli::Puzzle*)

#endif // PALAPELI_PUZZLE_H

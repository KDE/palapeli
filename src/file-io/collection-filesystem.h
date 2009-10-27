/***************************************************************************
 *   Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef PALAPELI_COLLECTION_FILESYSTEM_H
#define PALAPELI_COLLECTION_FILESYSTEM_H

#include "collection.h"

#include <QStringList>
class KUrl;

namespace Palapeli
{
	//This "meta collection" maintains temporary links to puzzle files that are not part of any collection. (This is necessary because all puzzle transactions operate on QModelIndexes.)
	class FileSystemCollection : public Palapeli::Collection
	{
		public:
			FileSystemCollection();

			QModelIndex providePuzzle(const KUrl& location);
			///Allow the user to select some puzzles from an arbitrary position.
			QModelIndexList selectPuzzles();

			//"Import" means: Export a puzzle from another collection to an arbitrary file
			virtual bool canImportPuzzles() const;
			virtual QModelIndex importPuzzle(const Palapeli::Puzzle* const puzzle);
		private:
			int indexOfPuzzle(const KUrl& location) const;
			QModelIndex addPuzzleInternal(const KUrl& location, Palapeli::Puzzle* puzzle);

			QStringList m_usedIdentifiers;
	};
}

#endif // PALAPELI_COLLECTION_FILESYSTEM_H

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

#ifndef PALAPELI_PUZZLE_H
#define PALAPELI_PUZZLE_H

#include "puzzlestructs.h"

#include <QFutureWatcher>
class KJob;
class KTempDir;
#include <KUrl>

namespace Palapeli
{

	class Puzzle : public QObject
	{
		Q_OBJECT
		public:
			Puzzle(const KUrl& location);
			Puzzle(const Palapeli::Puzzle& other);
			Puzzle(Palapeli::PuzzleMetadata* metadata, Palapeli::PuzzleContents* contents, Palapeli::PuzzleCreationContext* creationContext = 0);
			~Puzzle();

			KUrl location() const;
			void setLocation(const KUrl& location);

			static const QSize ThumbnailBaseSize;
			const Palapeli::PuzzleMetadata* metadata() const;
			const Palapeli::PuzzleContents* contents() const;

			///This can be used by a collection that provides metadata caching. Use only if you know what you're doing!
			void injectMetadata(Palapeli::PuzzleMetadata* metadata);
			///If metadata has already being read, this function will do nothing unless \a force is true.
			bool readMetadata(bool force = false);
			///If metadata has already being read, this function will do nothing unless \a force is true.
			bool readContents(bool force = false);
			bool write();
		private Q_SLOTS:
			void writeFinished(KJob* job);
			void finishWritingArchive();
		private:
			void createNewArchiveFile();

			KUrl m_location;
			KUrl m_loadLocation; //This is an optimization flag: If nothing has been changed after the puzzle has been loaded, then the write() method will only copy the original puzzle file from this location to the current location. When something is changed, m_loadLocation will be cleared.
			Palapeli::PuzzleMetadata* m_metadata;
			Palapeli::PuzzleContents* m_contents;
			Palapeli::PuzzleCreationContext* m_creationContext; //NOTE: This is NOT copied in copy-constructors.
			KTempDir* m_cache;
			QFutureWatcher<void> m_createArchiveWatcher;
	};
}

#endif // PALAPELI_PUZZLE_H

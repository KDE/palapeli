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

#ifndef PALAPELI_OLDPUZZLE_H
#define PALAPELI_OLDPUZZLE_H

#include "puzzlestructs.h"

#include <QFutureWatcher>
class KJob;
class KTempDir;
#include <KUrl>

namespace Palapeli
{
	class Puzzle;
	class PuzzleComponent;

	class OldPuzzle : public QObject
	{
		Q_OBJECT
		public:
			OldPuzzle(Palapeli::PuzzleComponent* mainComponent, const KUrl& location, const QString& identifier);
			OldPuzzle(const KUrl& location, const QString& identifier);
			OldPuzzle(const Palapeli::OldPuzzle& other, const QString& identifier);
			OldPuzzle(const Palapeli::PuzzleCreationContext& context, const QString& identifier);
			~OldPuzzle();

			KUrl location() const;
			void setLocation(const KUrl& location);

			Palapeli::Puzzle* newPuzzle() const;
			const Palapeli::PuzzleMetadata* metadata() const;
			const Palapeli::PuzzleContents* contents() const;

			///This can be used by a collection that provides metadata caching. Use only if you know what you're doing!
			void injectMetadata(Palapeli::PuzzleMetadata* metadata);
			///If metadata has already being read, this function will do nothing.
			bool readMetadata();
			///If metadata has already being read, this function will do nothing.
			bool readContents();
			bool write();
		Q_SIGNALS:
			void writeFinished();
		private Q_SLOTS:
			void writeFinished(KJob* job);
			void finishWritingArchive();
		private:
			void createNewArchiveFile();

			Palapeli::Puzzle* m_puzzle;
			KTempDir* m_cache;
			QFutureWatcher<void> m_createArchiveWatcher;
	};
}

#endif // PALAPELI_OLDPUZZLE_H

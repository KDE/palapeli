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

//TODO: write images

#include <QPixmap>
#include <KUrl>
class KDesktopFile;
class KTempDir;

namespace Palapeli
{
	class Puzzle
	{
		Q_DISABLE_COPY(Puzzle)
		public:
			Puzzle(const KUrl& locationUrl);
			~Puzzle();

			QString identifier() const;

			const KDesktopFile* manifest();
			QMap<int, QPixmap> pieces();
			QMap<int, QPoint> pieceOffsets();
			QList<QPair<int, int> > relations();
		protected:
			void loadArchive();
			void loadPuzzleContents();
		private:
			KUrl m_locationUrl;
			///When the puzzle is loaded, this directory contains the contents of the puzzle archive.
			KTempDir* m_cache;
			KDesktopFile* m_manifest;
			///These members are used as cache for the puzzle contents.
			QMap<int, QPixmap> m_pieces;
			QMap<int, QPoint> m_pieceOffsets;
			QList<QPair<int, int> > m_relations;
	};
}

#endif // PALAPELI_PUZZLE_H

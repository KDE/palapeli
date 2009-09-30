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

#include "puzzlelocation.h"

#include <QPixmap>
class KTempDir;

namespace Palapeli
{
	struct PuzzleMetadata
	{
		QString name, author, comment;
		int pieceCount;
		QPixmap thumbnail;
	};

	struct PuzzleContents
	{
		QSize imageSize;
		QMap<int, QPixmap> pieces;
		QMap<int, QPoint> pieceOffsets;
		QList<QPair<int, int> > relations;
	};

	class Puzzle
	{
		public:
			Puzzle(const Palapeli::PuzzleLocation& location);
			Puzzle(const Palapeli::Puzzle& other);
			~Puzzle();

			Palapeli::PuzzleLocation location() const;
			void setLocation(const Palapeli::PuzzleLocation& location);

			static const QSize ThumbnailBaseSize;
			const Palapeli::PuzzleMetadata* metadata() const;
			const Palapeli::PuzzleContents* contents() const;

			///If metadata has already being read, this function will do nothing unless \a force is true.
			bool readMetadata(bool force = false);
			///If metadata has already being read, this function will do nothing unless \a force is true.
			bool readContents(bool force = false);
			bool write();
		private:
			Palapeli::PuzzleLocation m_location;
			Palapeli::PuzzleMetadata* m_metadata;
			Palapeli::PuzzleContents* m_contents;
			KTempDir* m_cache;
	};
}

#endif // PALAPELI_PUZZLE_H

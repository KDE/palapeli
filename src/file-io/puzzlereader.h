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

#ifndef PALAPELI_PUZZLEREADER_H
#define PALAPELI_PUZZLEREADER_H

#include <QPixmap>
class KDesktopFile;
class KTempDir;
#include <KUrl>

namespace Palapeli
{
	class PuzzleReader
	{
		Q_DISABLE_COPY(PuzzleReader)
		public:
			explicit PuzzleReader(const QString& identifier); ///< Instance represents a puzzle from the library. Metadata is loaded from the library cache if possible.
			explicit PuzzleReader(const KUrl& locationUrl);
			~PuzzleReader();

			void loadArchive();
			void loadMetadata();
			void loadPuzzle();

			static const QSize ThumbnailBaseSize;
			//stuff that is available when the metadata has been loaded (maybe from the cache)
			QString identifier() const;
			QString name() const;
			QString author() const;
			QString comment() const;
			QPixmap thumbnail() const;
			//stuff that is only available after having loaded the puzzle
			QSize imageSize() const;
			QMap<int, QPixmap> pieces() const;
			QMap<int, QPoint> pieceOffsets() const;
			QList<QPair<int, int> > relations() const;
		private:
			KUrl m_locationUrl;
			bool m_metadataLoaded, m_puzzleLoaded;
			//caches for archive loading
			KTempDir* m_cache;
			KDesktopFile* m_manifest;
			//stuff that is available when the metadata has been loaded (maybe from the cache)
			QString m_name;
			QString m_author;
			QString m_comment;
			QPixmap m_thumbnail;
			//stuff that is only available after having loaded the puzzle
			QSize m_imageSize;
			QMap<int, QPixmap> m_pieces;
			QMap<int, QPoint> m_pieceOffsets;
			QList<QPair<int, int> > m_relations;
	};
}

#endif // PALAPELI_PUZZLEREADER_H

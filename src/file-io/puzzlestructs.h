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

#ifndef PALAPELI_PUZZLESTRUCTS_H
#define PALAPELI_PUZZLESTRUCTS_H

#include <QMap>
#include <QPair>
#include <QPixmap>

//NOTE: This header defines data structues for Palapeli::Puzzle. PuzzleMetadata contains a bunch of metadata, which is used in the Palapeli::Collection classes. PuzzleContents contains the constituents of the puzzle, and is used by the Palapeli::Scene. Both structures are separate, to allow Palapeli::Puzzle to load only the metadata if only they are needed. The PuzzleCreationContext structure contains some data which is generated during puzzle creation, but not loaded in the current Palapeli version. When writing a newly generated puzzle for the first time, they are also written into the file, to have them available if a newer Palapeli version decides to use them.

namespace Palapeli
{
	struct PuzzleMetadata
	{
		QString name, author, comment;
		int pieceCount;
		QImage thumbnail;
	};

	struct PuzzleContents
	{
		QSize imageSize;
		QMap<int, QPixmap> pieces;
		QMap<int, QPoint> pieceOffsets;
		QList<QPair<int, int> > relations;
	};

	struct PuzzleCreationContext
	{
		QByteArray usedSlicer;
		QMap<QByteArray, QVariant> usedSlicerArgs;
		QImage image;
		QMap<int, QImage> pieces; //The piece images are already in PuzzleContents, but we do also pass the QImage pictures because they have been generated anyway, and can be written in non-GUI threads.
	};
}

#endif // PALAPELI_PUZZLESTRUCTS_H

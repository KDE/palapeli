/*
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELI_PUZZLESTRUCTS_H
#define PALAPELI_PUZZLESTRUCTS_H

#include <QMap>
#include <QPair>
#include <QImage>
#include <QVariant>
//NOTE: This header defines data structures for Palapeli::Puzzle. PuzzleMetadata contains a bunch of metadata, which is used in the Palapeli::Collection classes. PuzzleContents contains the constituents of the puzzle, and is used by the Palapeli::Scene. Both structures are separate, to allow Palapeli::Puzzle to load only the metadata if only they are needed. The PuzzleCreationContext structure contains some data which is generated during puzzle creation, but not loaded in the current Palapeli version. When writing a newly generated puzzle for the first time, they are also written into the file, to have them available if a newer Palapeli version decides to use them.

namespace Palapeli
{
	struct PuzzleMetadata
	{
		QString name, author, comment;
		int pieceCount;
		QImage image, thumbnail;
		bool modifyProtection;

		static const QSize ThumbnailBaseSize;
	};

	struct PuzzleContents
	{
		QSize imageSize;
		QMap<int, QImage> pieces;
		QMap<int, QPoint> pieceOffsets;
		QList<QPair<int, int> > relations;
	};

	struct PuzzleCreationContext : public Palapeli::PuzzleMetadata
	{
		QString slicer;
		QByteArray slicerMode;
		QMap<QByteArray, QVariant> slicerArgs;
	};
}

#endif // PALAPELI_PUZZLESTRUCTS_H

/*
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "slicer-rect.h"

#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(RectSlicer, "palapeli_rectslicer.json")

RectSlicer::RectSlicer(QObject* parent, const QVariantList& args)
	: Pala::Slicer(parent, args)
	, Pala::SimpleGridPropertySet(this)
{
}

bool RectSlicer::run(Pala::SlicerJob* job)
{
	//read job
	const QSize pieceCount = Pala::SimpleGridPropertySet::pieceCount(job);
	const int xCount = pieceCount.width();
	const int yCount = pieceCount.height();
	const QImage image = job->image();
	//calculate some metrics
	const int pieceWidth = image.width() / xCount;
	const int pieceHeight = image.height() / yCount;
	const int roundingErrorX = image.width() - pieceWidth * xCount;
	const int roundingErrorY = image.height() - pieceHeight * yCount;
	//create pieces
	for (int x = 0; x < xCount; ++x)
	{
		for (int y = 0; y < yCount; ++y)
		{
			//calculate more metrics
			const QPoint offset(x * pieceWidth, y * pieceHeight);
			const int thisPieceWidth = (x == xCount) ? pieceWidth + roundingErrorX : pieceWidth;
			const int thisPieceHeight = (y == yCount) ? pieceHeight + roundingErrorY : pieceHeight;
			const QSize pieceSize(thisPieceWidth, thisPieceHeight);
			//copy image part to piece
			const QRect pieceBounds(offset, pieceSize);
			const QImage pieceImage = image.copy(pieceBounds);
			job->addPiece(x + y * xCount, pieceImage, offset);
		}
	}
	//create relations
	for (int x = 0; x < xCount; ++x)
	{
		for (int y = 0; y < yCount; ++y)
		{
			//along X axis (pointing left)
			if (x != 0)
				job->addRelation(x + y * xCount, (x - 1) + y * xCount);
			//along Y axis (pointing up)
			if (y != 0)
				job->addRelation(x + y * xCount, x + (y - 1) * xCount);
		}
	}
	return true;
}

#include "slicer-rect.moc"

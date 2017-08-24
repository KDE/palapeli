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

#include "slicer-rect.h"

#include <KLocalizedString>
#include <KPluginFactory>

K_PLUGIN_FACTORY(RectSlicerFactory, registerPlugin<RectSlicer>();)

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

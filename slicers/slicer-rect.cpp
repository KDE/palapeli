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
#include <KPluginLoader>

K_PLUGIN_FACTORY(RectSlicerFactory, registerPlugin<RectSlicer>();)
K_EXPORT_PLUGIN(RectSlicerFactory("palapeli_rectslicer"))

RectSlicer::RectSlicer(QObject* parent, const QVariantList& args)
	: Palapeli::Slicer(parent, args)
{
	Palapeli::SlicerProperty* prop;
	prop = new Palapeli::SlicerProperty(Palapeli::SlicerProperty::Integer, i18n("Piece count in horizontal direction"));
	prop->setRange(3, 100);
	prop->setDefaultValue(10);
	addProperty("XCount", prop);
	prop = new Palapeli::SlicerProperty(Palapeli::SlicerProperty::Integer, i18n("Piece count in vertical direction"));
	prop->setRange(3, 100);
	prop->setDefaultValue(10);
	addProperty("YCount", prop);
}

void RectSlicer::run(Palapeli::SlicerJob* job)
{
	//read job
	const int xCount = job->argument("XCount").toInt();
	const int yCount = job->argument("YCount").toInt();
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
			job->addPiece(x * yCount + y, pieceImage, offset);
		}
	}
	//create relations
	for (int x = 0; x < xCount; ++x)
	{
		for (int y = 0; y < yCount; ++y)
		{
			//along X axis (pointing left)
			if (x != 0)
				job->addRelation(x * yCount + y, (x - 1) * yCount + y);
			//along Y axis (pointing up)
			if (y != 0)
				job->addRelation(x * yCount + y, x * yCount + (y - 1));
		}
	}
}

#include "slicer-rect.moc"
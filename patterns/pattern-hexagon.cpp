/***************************************************************************
 *   Copyright (C) 2008 Stefan Majewsky <majewsky@gmx.net>
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

#include "pattern-hexagon.h"

#include <cmath>
#include <QImage>
#include <QPainter>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginLoader>

K_PLUGIN_FACTORY(HexagonalPatternFactory, registerPlugin<Palapeli::HexagonalPatternPlugin>();)
K_EXPORT_PLUGIN(HexagonalPatternFactory("palapeli_hexagonpattern"))

//General implementation note: The slicing algorithm implemented in this plugin is not optimised because I attempt to generalise it into the generic "regular pattern" algorithm later.

//BEGIN Palapeli::HexagonalPattern

Palapeli::HexagonalPattern::HexagonalPattern(int xCount, int yCount)
	: Palapeli::Pattern()
	, m_xCount(qMax(1, xCount))
	, m_yCount(qMax(1, yCount))
{
}

Palapeli::HexagonalPattern::~HexagonalPattern()
{
}

int Palapeli::HexagonalPattern::estimatePieceCount() const
{
	return m_xCount * m_yCount;
}

void Palapeli::HexagonalPattern::doSlice(const QImage& image)
{
	const int width = image.width(), height = image.height();
	const QSize imageSize(width, height);
	const QRect imageRect(0, 0, width, height);
	const int pieceWidth = width / m_xCount + 1, pieceHeight = height / m_yCount + 1; //the +1 is a hack to avoid pieces with width or height = 1
	const QSize pieceSize(pieceWidth, pieceHeight);
	const QRect pieceRect(0, 0, pieceWidth, pieceHeight);
	//block 1: the hexagonal mask
	//create painter path for mask - start at left, then go around clockwise
	QPainterPath path;
	path.moveTo(0, pieceHeight / 2);
	path.lineTo(pieceWidth / 4, 0);
	path.lineTo(pieceWidth * 3 / 4, 0);
	path.lineTo(pieceWidth, pieceHeight / 2);
	path.lineTo(pieceWidth * 3 / 4, pieceHeight);
	path.lineTo(pieceWidth / 4, pieceHeight);
	path.lineTo(0, pieceHeight / 2);
	//create mask
	QImage mask(pieceSize, QImage::Format_ARGB32_Premultiplied);
	mask.fill(0x00000000);
	QPainter painter;
	painter.begin(&mask);
	painter.setPen(Qt::NoPen);
	painter.setBrush(Qt::black);
	painter.drawPath(path);
	painter.end();
	//block 2: create pieces
	//Note that the pieces are ordered linearly in y direction, but not in x direction; i.e. (x,y + 1) and (x,y) have the same position in X direction, but (x + 1,y) and (x,y). That is the main characteristic of a hexagonal grid: One of the three main axes of this grid can equal a normal carthesic axis (the Y axis in this case), but the other carthesic axis is not an axis in this grid.
	QList<QPoint> pieceIndices; //index (0,0) is at the center
	//get (approximate) coordinate range for pieces
	int maxX = 0, maxY = 0;
	for (int x = 0; imageRect.intersects(pieceRect.translated(pieceBasePosition(x, 0, pieceSize, imageSize))); ++x)
	{
		for (int y = 0; imageRect.intersects(pieceRect.translated(pieceBasePosition(x, y, pieceSize, imageSize))); ++y)
		{
			maxX = qMax(x, maxX);
			maxY = qMax(y, maxY);
		}
	}
	//iterate through possible pieces
	for (int x = -maxX; x <= maxX; ++x)
	{
		for (int y = -maxY; y <= maxY; ++y)
		{
			//create mask for this piece - TODO: optimise this
			const QPoint piecePosition = pieceBasePosition(x, y, pieceSize, imageSize);
			if (!pieceRect.translated(piecePosition).intersects(imageRect))
				continue;
			QImage thisPiece = image.copy(pieceRect.translated(piecePosition));
			painter.begin(&thisPiece);
			painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
			painter.drawImage(pieceRect, mask, pieceRect);
			painter.end();
			addPiece(thisPiece, pieceRect.translated(piecePosition));
			pieceIndices << QPoint(x, y);
		}
	}
	//block 3: add relations
	int index1, index2;
	for (int x = -maxX; x <= maxX; ++x)
	{
		for (int y = -maxY; y <= maxY; ++y)
		{
			index1 = pieceIndices.indexOf(QPoint(x, y));
			if (index1 == -1)
				continue;
			//vertical axis
			index2 = pieceIndices.indexOf(QPoint(x, y + 1));
			if (index2 != -1)
				addRelation(index1, index2);
			index2 = pieceIndices.indexOf(QPoint(x, y - 1));
			if (index2 != -1)
				addRelation(index1, index2);
			//first diagonal axis
			index2 = pieceIndices.indexOf(QPoint(x + 1, y));
			if (index2 != -1)
				addRelation(index1, index2);
			index2 = pieceIndices.indexOf(QPoint(x - 1, y));
			if (index2 != -1)
				addRelation(index1, index2);
			//second diagonal axis
			const int xFactor = qAbs(x) % 2 * 2 - 1; //-1 for even and +1 for odd values of qAbs(x)
			index2 = pieceIndices.indexOf(QPoint(x + 1, y - xFactor));
			if (index2 != -1)
				addRelation(index1, index2);
			index2 = pieceIndices.indexOf(QPoint(x - 1, y - xFactor));
			if (index2 != -1)
				addRelation(index1, index2);
		}
	}
}

QPoint Palapeli::HexagonalPattern::pieceBasePosition(int x, int y, const QSize& piece, const QSize& image) const
{
	//piece (0,0) has its center at (image.width() / 2, image.height() / 2)
	const QPoint centerPiece((image.width() - piece.width()) / 2, (image.height() - piece.height()) / 2);
	QPoint thisPiece(centerPiece.x() + x * piece.width() * 3 / 4, centerPiece.y() + y * piece.height()); //factor 3/4 because pieces do not need their full width in X direction
	if (x % 2) //short for: x % 2 == 1
		thisPiece.ry() -= piece.height() / 2; //move pieces up if x is odd
	return thisPiece;
}

//END Palapeli::HexagonalPattern

//BEGIN Palapeli::HexagonalPatternConfiguration

Palapeli::HexagonalPatternConfiguration::HexagonalPatternConfiguration(const QString& pluginName, const QString& displayName)
	: Palapeli::PatternConfiguration(pluginName, displayName)
{
	//add properties
	addProperty("xcount", Palapeli::PatternConfiguration::Integer, i18n("Piece count in horizontal direction:"));
	addProperty("YCount", Palapeli::PatternConfiguration::Integer, i18n("Piece count in vertical direction:"));
	//set parameters (minimum and maximum in this case)
	QVariantList params; params << 3 << 100;
	addPropertyParameters("xcount", params);
	addPropertyParameters("YCount", params);
	//set default values
	setProperty("xcount", 10);
	setProperty("YCount", 10);
}

Palapeli::HexagonalPatternConfiguration::~HexagonalPatternConfiguration()
{
}

Palapeli::Pattern* Palapeli::HexagonalPatternConfiguration::createPattern() const
{
	return new Palapeli::HexagonalPattern(property("xcount").toInt(), property("YCount").toInt());
}

//END Palapeli::HexagonalPatternConfiguration

//BEGIN Palapeli::HexagonalPatternPlugin

Palapeli::HexagonalPatternPlugin::HexagonalPatternPlugin(QObject* parent, const QVariantList& args)
	: Palapeli::PatternPlugin(parent, args)
{
}

Palapeli::HexagonalPatternPlugin::~HexagonalPatternPlugin()
{
}

QList<Palapeli::PatternConfiguration*> Palapeli::HexagonalPatternPlugin::createInstances() const
{
	QList<Palapeli::PatternConfiguration*> list;
	list << new Palapeli::HexagonalPatternConfiguration(pluginName(), displayName());
	return list;
}

//END Palapeli::HexagonalPatternPlugin

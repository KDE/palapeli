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

#include "pattern-rectangular.h"

#include <QImage>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginLoader>

K_PLUGIN_FACTORY(RectangularPatternFactory, registerPlugin<Palapeli::RectangularPatternPlugin>();)
K_EXPORT_PLUGIN(RectangularPatternFactory("palapeli_rectangularpattern"))

//BEGIN Palapeli::RectangularPattern

Palapeli::RectangularPattern::RectangularPattern(int xCount, int yCount)
	: Palapeli::Pattern()
	, m_xCount(qMax(1, xCount))
	, m_yCount(qMax(1, yCount))
{
}

Palapeli::RectangularPattern::~RectangularPattern()
{
}

void Palapeli::RectangularPattern::doSlice(const QImage& image)
{
	reportPieceCount(m_xCount * m_yCount);
	int width = image.width(), height = image.height();
	int pieceWidth = width / m_xCount, pieceHeight = height / m_yCount;
	//make pieces
	for (int x = 0; x < m_xCount; ++x)
	{
		for (int y = 0; y < m_yCount; ++y)
		{
			const int thisPieceWidth = (x + 1 == m_xCount) ? pieceWidth : pieceWidth + 1;
			const int thisPieceHeight = (y + 1 == m_yCount) ? pieceHeight : pieceHeight + 1;
			//the +1 makes the pieces overlap, therefore preventing infitesimally small spaces between the pieces resulting in white lines; but we have to check if the right and bottom edge are still inside of the image (or on its border)
			QRectF pieceRect(x * pieceWidth, y * pieceHeight, thisPieceWidth, thisPieceHeight);
			addPiece(image.copy(pieceRect.toRect()), pieceRect);
		}
	}
	//build relationships between pieces
	for (int x = 0; x < m_xCount; ++x)
	{
		for (int y = 0; y < m_yCount; ++y)
		{
			//along X axis (pointing left)
			if (x != 0)
				addRelation(x * m_yCount + y, (x - 1) * m_yCount + y);
			//along Y axis (pointing up)
			if (y != 0)
				addRelation(x * m_yCount + y, x * m_yCount + (y - 1));
		}
	}
}

//END Palapeli::RectangularPattern

//BEGIN Palapeli::RectangularPatternConfiguration

Palapeli::RectangularPatternConfiguration::RectangularPatternConfiguration(const QString& pluginName, const QString& displayName, const QString& iconName)
	: Palapeli::PatternConfiguration(pluginName, displayName, iconName)
{
	//add properties
	addProperty("XCount", Palapeli::PatternConfiguration::Integer, i18n("Piece count in horizontal direction:"));
	addProperty("YCount", Palapeli::PatternConfiguration::Integer, i18n("Piece count in vertical direction:"));
	//set parameters (minimum and maximum in this case)
	QVariantList params; params << 3 << 100;
	addPropertyParameters("XCount", params);
	addPropertyParameters("YCount", params);
	//set default values
	setProperty("XCount", 10);
	setProperty("YCount", 10);
}

Palapeli::RectangularPatternConfiguration::~RectangularPatternConfiguration()
{
}

Palapeli::Pattern* Palapeli::RectangularPatternConfiguration::createPattern() const
{
	return new Palapeli::RectangularPattern(property("XCount").toInt(), property("YCount").toInt());
}

//END Palapeli::RectangularPatternConfiguration

//BEGIN Palapeli::RectangularPatternPlugin

Palapeli::RectangularPatternPlugin::RectangularPatternPlugin(QObject* parent, const QVariantList& args)
	: Palapeli::PatternPlugin(parent, args)
{
}

Palapeli::RectangularPatternPlugin::~RectangularPatternPlugin()
{
}

QList<Palapeli::PatternConfiguration*> Palapeli::RectangularPatternPlugin::createInstances() const
{
	QList<Palapeli::PatternConfiguration*> list;
	list << new Palapeli::RectangularPatternConfiguration(pluginName(), displayName(), iconName());
	return list;
}

//END Palapeli::RectangularPatternPlugin

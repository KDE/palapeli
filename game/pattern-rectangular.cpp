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

//ATTENTION: This code is part of the new pattern implementation which is not included in the build yet (because there is no UI code to make use of it). It may therefore not compile correctly.

#include "pattern-rect.h"

#include <QPainter>
#include <KLocalizedString>

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

void Palapeli::RectangularPattern::slice(const QImage& image)
{
	QList<Palapeli::Piece*> pieces;
	int width = image.width(), height = image.height();
	int pieceWidth = width / m_xCount, pieceHeight = height / m_yCount;
	//make pieces
	for (int x = 0; x < m_xCount; ++x)
	{
		for (int y = 0; y < m_yCount; ++y)
		{
			QPixmap pix(pieceWidth, pieceHeight);
			QPainter painter(&pix);
			painter.drawImage(QPoint(0, 0), image, QRect(x * pieceWidth, y * pieceHeight, pieceWidth, pieceHeight));
			painter.end();
			addPiece(pix, QRectF(x * pieceWidth, y * pieceHeight, pieceWidth, pieceHeight));
		}
	}
	//build relationships between pieces
	for (int x = 0; x < m_xCount; ++x)
	{
		for (int y = 0; y < m_yCount; ++y)
		{
			//left
			if (x != 0)
				addRelation(x * m_yCount + y, (x - 1) * m_yCount + y, QPointF(-pieceWidth, 0));
			//right
			if (x != m_xCount - 1)
				addRelation(x * m_yCount + y, (x + 1) * m_yCount + y, QPointF(pieceWidth, 0));
			//top
			if (y != 0)
				addRelation(x * m_yCount + y, x * m_yCount + (y - 1), QPointF(0, -pieceHeight));
			//bottom
			if (y != m_yCount - 1)
				addRelation(x * m_yCount + y, x * m_yCount + (y + 1), QPointF(0, pieceHeight));
		}
	}
}

//END Palapeli::RectangularPattern

//BEGIN Palapeli::RectangularPatternConfiguration

Palapeli::RectangularPatternConfiguration::RectangularPatternConfiguration()
	: Palapeli::PatternConfiguration("rectangular", i18n("Simple rectangles"))
{
	setSizeDefinitionMode(Palapeli::PatternConfiguration::CountSizeDefinition);
}

Palapeli::RectangularPatternConfiguration::~RectangularPatternConfiguration()
{
}

void Palapeli::RectangularPatternConfiguration::readArguments(KConfigGroup* config)
{
	readArgumentsBase(config);
}

void Palapeli::RectangularPatternConfiguration::writeArguments(KConfigGroup* config) const
{
	writeArgumentsBase(config);
}

Palapeli::Pattern* Palapeli::RectangularPatternConfiguration::createPattern() const
{
	return new Palapeli::RectangularPattern(xCount(), yCount());
}

//END Palapeli::RectangularPatternConfiguration

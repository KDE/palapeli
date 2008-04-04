/***************************************************************************
 *   Copyright (C) 2008 Felix Lemke <lemke.felix@ages-skripte.org>
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

#include "pattern-rect.h"
#include "piece.h"
#include "scene.h"

#include <QPainter>

Palapeli::RectangularPattern::RectangularPattern(const QStringList& arguments)
	: Palapeli::Pattern(arguments)
	, m_xCount(10)
	, m_yCount(10)
{
	//try to read arguments
	foreach (QString argument, arguments)
	{
		QStringList data = argument.split('=');
		if (data.count() != 2)
			continue;
		QString title = data[0];
		if (title == "XCount")
		{
			//try to convert value to integer; reset to default if conversion fails
			bool isNumeric = false;
			m_xCount = data[1].toInt(&isNumeric);
			if (!isNumeric)
				m_xCount = 10;
		}
		else if (title == "YCount")
		{
			bool isNumeric = false;
			m_yCount = data[1].toInt(&isNumeric);
			if (!isNumeric)
				m_yCount = 10;
		}
	}
}

Palapeli::RectangularPattern::RectangularPattern(int xCount, int yCount)
	: Palapeli::Pattern(QStringList())
	, m_xCount(qMax(1, xCount))
	, m_yCount(qMax(1, yCount))
{
}

Palapeli::RectangularPattern::~RectangularPattern()
{
}

QString Palapeli::RectangularPattern::name() const
{
	return "rectangular";
}

QStringList Palapeli::RectangularPattern::arguments() const
{
	const QString xCount("XCount=%1"), yCount("YCount=%1");
	return QStringList() << xCount.arg(m_xCount) << yCount.arg(m_yCount);
}

QList<Palapeli::Piece*> Palapeli::RectangularPattern::slice(const QImage& image, Palapeli::Scene* scene)
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
			Palapeli::Piece* piece = new Palapeli::Piece(pix, scene, pieceWidth, pieceHeight);
			pieces << piece;
		}
	}
	//build relationships between pieces
	for (int x = 0; x < m_xCount; ++x)
	{
		for (int y = 0; y < m_yCount; ++y)
		{
			//left
			if (x != 0)
				pieces[x * m_yCount + y]->addNeighbor(pieces[(x - 1) * m_yCount + y], -pieceWidth, 0);
			//right
			if (x != m_xCount - 1)
				pieces[x * m_yCount + y]->addNeighbor(pieces[(x + 1) * m_yCount + y], pieceWidth, 0);
			//top
			if (y != 0)
				pieces[x * m_yCount + y]->addNeighbor(pieces[x * m_yCount + (y - 1)], 0, -pieceHeight);
			//bottom
			if (y != m_yCount - 1)
				pieces[x * m_yCount + y]->addNeighbor(pieces[x * m_yCount + (y + 1)], 0, pieceHeight);
		}
	}
	return pieces;
}

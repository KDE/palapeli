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
#include "manager.h"
#include "piece.h"

#include <QPainter>
#include <KConfigGroup>

Palapeli::RectangularPattern::RectangularPattern(KConfigGroup* arguments, Manager* manager)
	: Palapeli::Pattern(arguments, manager)
	, m_xCount(arguments->readEntry("XCount", 10))
	, m_yCount(arguments->readEntry("YCount", 10))
{
}

Palapeli::RectangularPattern::RectangularPattern(int xCount, int yCount, Manager* manager)
	: Palapeli::Pattern(0, manager)
	, m_xCount(qMax(1, xCount))
	, m_yCount(qMax(1, yCount))
{
}

Palapeli::RectangularPattern::~RectangularPattern()
{
}

QString Palapeli::RectangularPattern::name() const
{
	return QLatin1String("rectangular");
}

void Palapeli::RectangularPattern::writeArguments(KConfigGroup* target) const
{
	target->writeEntry("XCount", m_xCount);
	target->writeEntry("YCount", m_yCount);
}

QList<Palapeli::Piece*> Palapeli::RectangularPattern::slice(const QImage& image)
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
			Palapeli::Piece* piece = new Palapeli::Piece(pix, QSize(pieceWidth, pieceHeight), QPointF(x * pieceWidth, y * pieceHeight), m_manager);
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
				m_manager->addRelation(pieces[x * m_yCount + y], pieces[(x - 1) * m_yCount + y], QPointF(-pieceWidth, 0));
			//right
			if (x != m_xCount - 1)
				m_manager->addRelation(pieces[x * m_yCount + y], pieces[(x + 1) * m_yCount + y], QPointF(pieceWidth, 0));
			//top
			if (y != 0)
				m_manager->addRelation(pieces[x * m_yCount + y], pieces[x * m_yCount + (y - 1)], QPointF(0, -pieceHeight));
			//bottom
			if (y != m_yCount - 1)
				m_manager->addRelation(pieces[x * m_yCount + y], pieces[x * m_yCount + (y + 1)], QPointF(0, pieceHeight));
		}
	}
	return pieces;
}

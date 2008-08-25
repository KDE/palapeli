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

#ifndef PALADESIGN_ALGEBRA_H
#define PALADESIGN_ALGEBRA_H

class QLineF;
#include <QPointF>

namespace Paladesign
{

	namespace Algebra
	{
		qreal vectorLength(const QPointF& point);
		qreal distance(const QPointF& point1, const QPointF& point2);
		qreal distance(const QPointF& point, const QLineF& line, bool isLineSegment = false);
		QPointF mirrored(const QPointF& point, const QLineF& axis);

		int greatestCommonDivisor(int a, int b);
	}

}

qreal operator*(const QPointF& point1, const QPointF& point2);
QPointF operator!(const QPointF& point); //returns the vector that is orthogonal to this one

#endif // PALADESIGN_ALGEBRA_H

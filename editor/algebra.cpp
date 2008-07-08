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

#include "algebra.h"

#include <math.h>
#include <QLineF>

qreal Paladesign::Algebra::vectorLength(const QPointF& point)
{
	const qreal x = point.x(), y = point.y();
	return sqrt(x * x + y * y);
}

qreal Paladesign::Algebra::distance(const QPointF& point1, const QPointF& point2)
{
	return vectorLength(point2 - point1);
}

qreal Paladesign::Algebra::distance(const QPointF& point, const QLineF& line, bool isLineSegment)
{
	const QPointF lineEnd1 = line.p1(), lineEnd2 = line.p2();
	const QPointF lineVector = lineEnd2 - lineEnd1;
	//project distance vector from pointOnLine1 to point to the line
	const QPointF unitVector = lineVector / vectorLength(lineVector);
	const QPointF distanceVector = point - lineEnd1;
	const QPointF projectedDistanceVector = (distanceVector * unitVector) * unitVector;
	//projectedDistanceVector points to projection of point on line; find parameter for this foot point
	const qreal projectedPositionParameter = projectedDistanceVector.x() / lineVector.x();
	if (isLineSegment)
	{
		if (projectedPositionParameter <= 0)
			//point is at the far "left" of the line - return distance to "left" end
			return vectorLength(distanceVector);
		else if (projectedPositionParameter >= 1)
			//like above - point is at the far "right" of the line
			return vectorLength(point - lineEnd2);
	}
	//return distance to foot point
	return vectorLength(distanceVector - projectedDistanceVector);
}

QPointF Paladesign::Algebra::mirrored(const QPointF& point, const QLineF& axis)
{
	const QPointF lineEnd1 = axis.p1(), lineEnd2 = axis.p2();
	const QPointF lineVector = lineEnd2 - lineEnd1;
	//project distance vector from pointOnLine1 to point to the line
	const QPointF unitVector = lineVector / vectorLength(lineVector);
	const QPointF distanceVector = point - lineEnd1;
	const QPointF projectedDistanceVector = (distanceVector * unitVector) * unitVector;
	const QPointF orthogonalLineToPointVector = distanceVector - projectedDistanceVector;
	//subtract orthogonalLineToPointVector from point once to get the projection's foot point, twice to get the image point
	return point - 2.0 * orthogonalLineToPointVector;
}

int Paladesign::Algebra::greatestCommonDivisor(int a, int b)
{
	//TODO: Find efficient implementation and actual name.
	int smaller = qMin(a, b);
	int gcd = 1;
	for (int i = 2; i <= smaller; ++i)
	{
		if (a % i + b % i == 0)
			gcd = i;
	}
	return gcd;
}

qreal operator*(const QPointF& point1, const QPointF& point2)
{
	return point1.x() * point2.x() + point1.y() * point2.y();
}

QPointF operator!(const QPointF& point)
{
	return QPointF(point.y(), -point.x());
}

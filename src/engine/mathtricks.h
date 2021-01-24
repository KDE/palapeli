/*
    SPDX-FileCopyrightText: 2011 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELI_ENGINE_MATHTRICKS_H
#define PALAPELI_ENGINE_MATHTRICKS_H

#include <QtMath>

namespace Palapeli
{
	template<typename T> struct fastnorm_helper { typedef T fp; };
	template<> struct fastnorm_helper<int> { typedef qreal fp; };

	//Approximates sqrt(x^2 + y^2) with a maximum error of 4%.
	//Speedup on my machine is 30% for unoptimized and up to 50% for optimized builds.
	template<typename T> inline T fastnorm(T x, T y)
	{
		//need absolute values for distance measurement
		x = qAbs<T>(x); y = qAbs<T>(y);
		//There are two simple approximations to sqrt(x^2 + y^2):
		//  max(x, y) -> good for x >> y or x << y          (square-shaped unit circle)
		//  1/sqrt(2) * (x + y) -> good for x = y (approx.) (diamond-shaped unit circle)
		//The worse approximation is always bigger. By taking the maximum of both,
		//we get an octagon-shaped upper approximation of the unit circle. The
		//approximation can be refined by a prefactor (called a) below.
		typedef typename fastnorm_helper<T>::fp fp;
		static const fp a = (1 + sqrt(4 - 2 * qSqrt(2))) / 2;
		static const fp b = qSqrt(0.5);
		const T metricOne = qMax<T>(x, y);
		const T metricTwo = b * (x + y);
		return a * qMax(metricOne, metricTwo);
		//idea for this function from http://www.azillionmonkeys.com/qed/sqroot.html
		//(though the equations there have some errors at the time of this writing)
	}

	static inline int fastnorm(const QPoint& p)
	{
		return fastnorm<int>(p.x(), p.y());
	}
	static inline qreal fastnorm(const QPointF& p)
	{
		return fastnorm<qreal>(p.x(), p.y());
	}

	//Approximates cos(angle * M_2PI / 256) with quadratic functions.
	static inline qreal hexcos(int angle)
	{
		// project angle into [0, 255]
		angle = angle & 0xff;
		// project angle into [-64, 63] (where negative angles counter-intuitively
		// represent negative results)
		if (angle >= 128)
			angle = 255 - angle;
		if (angle >= 64)
			angle = angle-128;
		//approximate cosine arcs with parabola
		const qreal result = 1 - qreal(angle * angle) / (64.0 * 64.0);
		return angle < 0 ? -result : result;
	}
}

#endif // PALAPELI_ENGINE_MATHTRICKS_H

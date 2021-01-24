/*
    SPDX-FileCopyrightText: 2010 Johannes Loehnert <loehnert.kde@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "utilities.h"

#include <QDebug>
#include <QRandomGenerator>

void getBestFit(int &xCount, int &yCount, qreal target_aspect, int approx_count) {
    qreal nx_exact = sqrt(approx_count * target_aspect);
    // avoid taking the sqrt again
    qreal ny_exact = approx_count / nx_exact;

    // catch very odd cases
    if (nx_exact < 1) nx_exact = 1.01;
    if (ny_exact < 1) ny_exact = 1.01;

    qreal aspect1 = floor(nx_exact) / ceil(ny_exact);
    qreal aspect2 = ceil(nx_exact) / floor(ny_exact);

    aspect1 = target_aspect - aspect1;
    aspect2 = aspect2 - target_aspect;

    if (aspect1 < aspect2) ny_exact += 1.0; else nx_exact += 1.0;

    xCount = (int)(floor(nx_exact) + 0.1);
    yCount = (int)(floor(ny_exact) + 0.1);
}

void getBestFitExtended(int &xCount, int &yCount, qreal target_aspect, int approx_count,
            qreal tiles_per_cell, qreal additional_tiles_per_row, qreal additional_tiles_per_column, qreal additional_tiles) {
    // solves the equations
    //  N = TPC * x * y  +  ATPC * x + ATPR * y + AT
    //  target_aspect = x / y
    //
    // for x, y; and rounds them to the nearest integer values giving least distance to target_aspect.
    qreal p_half = (target_aspect * additional_tiles_per_column + additional_tiles_per_row) / (2 * target_aspect * tiles_per_cell);
    qreal q = (approx_count - additional_tiles) / (target_aspect * tiles_per_cell);
    
    qreal p_half_sq = p_half*p_half;
    if (p_half_sq + q < 0) {
        xCount = 1;
        yCount = 1;
        return;
    }
    
    qreal ny_exact = -p_half + sqrt(p_half_sq + q);
    qreal nx_exact = target_aspect * ny_exact;

    qDebug() << "nx_exact: " << nx_exact << " ny_exact: " << ny_exact <<
                "giving count: " << (tiles_per_cell*nx_exact*ny_exact + additional_tiles_per_column * nx_exact + additional_tiles_per_row * ny_exact + additional_tiles);

    // catch very odd cases
    if (nx_exact < 1) nx_exact = 1.01;
    if (ny_exact < 1) ny_exact = 1.01;

    qreal aspect1 = floor(nx_exact) / ceil(ny_exact);
    qreal aspect2 = ceil(nx_exact) / floor(ny_exact);
    qreal aspect3 = ceil(nx_exact) / ceil(ny_exact);

    aspect1 = target_aspect - aspect1;
    aspect2 = aspect2 - target_aspect;
    aspect3 = abs(aspect3 - target_aspect);

    if (aspect1 < aspect2) {
        ny_exact += 1.0; 
        if (aspect3 < aspect1) nx_exact += 1.0;
    }
    else {
        nx_exact += 1.0;
        if (aspect3 < aspect2) ny_exact += 1.0;
    }

    xCount = (int)(floor(nx_exact) + 0.1);
    yCount = (int)(floor(ny_exact) + 0.1);
}

// skews x with "strength" a.
// x is expected to lie within [0, 1].
// a = +/-1 is already a very strong skew.
// negative a: skew towards x=0, positive: skew towards x=1.
qreal skew_randnum(qreal x, qreal a) {
    if (a==0) return x;

    qreal asq = exp(-2 * abs(a));
    if (a>0) x = 1-x;

    qreal mp2 = (x-1) * (2/asq - 1);
    qreal q = (x-1)*(x-1) - 1;

    // We apply a function on x, which is a hyperbola through (0,0) and (1,1)
    // with (1, 0) as focal point. You really don't want to know the gory details.
    x = mp2 + sqrt(mp2*mp2 - q);

    if (a>0) x = 1-x;
    return x;
}


qreal nonuniform_rand(qreal min, qreal max, qreal sigma, qreal skew) {

    // 0.4247: sigma at which distribution function is 1/2 of max at interval boundaries

    qreal randNum;

    auto *generator = QRandomGenerator::global();
    if (sigma > 0.4247) {
        // "wide" distribution, use rejection sampling
        qreal x, y;
        qreal ssq = 2 * sigma * sigma;
        do {
            x = 0.000001 * qreal(generator->bounded(1000000));
            y = 0.000001 * qreal(generator->bounded(1000000));
        } while (y > exp(-(x-0.5)*(x-0.5)/ssq));

        randNum = x;
    }
    else {
        // "narrow" distribution, use Marsaglia method until a random number within 0, 1 is found.
        qreal u1, u2, p, q, x1, x2;

        randNum = -1;
        do {
            do {
                u1 = 0.000002 * qreal(generator->bounded(1000000)) - 1;
                u2 = 0.000002 * qreal(generator->bounded(1000000)) - 1;
                q = u1*u1 + u2*u2;
            } while (q>1);
            p = sqrt(-2 * log(q) / q) * sigma;
            x1 = u1 * p + 0.5; 
            x2 = u2 * p + 0.5;

            if (x1>= 0 && x1 <= 1) {
                randNum = x1;
            }
            else {
                if (x2 >= 0 && x2 <= 1) randNum = x2;
            }
        } while (randNum < 0);
    }

    return min + (max - min) * skew_randnum(randNum, skew);
}


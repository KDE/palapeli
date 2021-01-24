/*
    SPDX-FileCopyrightText: 2010 Johannes Loehnert <loehnert.kde@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELISLICERS_GOLDBERG_UTILITIES_H
#define PALAPELISLICERS_GOLDBERG_UTILITIES_H

#include <cmath>
#include <QImage>

inline qreal dotproduct(const QPointF& a, const QPointF& b) { return a.x() * b.x() + a.y() * b.y(); }

inline qreal dsin(qreal angle) { return sin(angle* M_PI / 180.0); }
inline qreal dcos(qreal angle) { return cos(angle* M_PI / 180.0); }


// get recommended cell count for the given aspect.
// aspect is image_width / image_height / (cell_aspect)
//  with cell_aspect = cell_width / cell_height (for non-square grid cell).
// sets xCount, yCount.
void getBestFit(int &xCount, int &yCount, qreal target_aspect, int approx_count); 
void getBestFitExtended(int &xCount, int &yCount, qreal target_aspect, int approx_count,
            qreal tiles_per_cell, qreal additional_tiles_per_row, qreal additional_tiles_per_column, qreal additional_tiles);

//A modified version of QImage::copy, which avoids rendering errors even if rect is outside the bounds of the source image.
QImage safeQImageCopy(const QImage& source, const QRect& rect);

// skews x with "strength" a.
// x is expected to lie within [0, 1].
// a = +/-1 is already a very strong skew.
// negative a: skew towards x=0, positive: skew towards x=1.
qreal skew_randnum(qreal x, qreal a);

qreal nonuniform_rand(qreal min, qreal max, qreal sigma, qreal skew);


#endif // PALAPELISLICERS_GOLDBERG_UTILITIES_H

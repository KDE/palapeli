/***************************************************************************
 *   Copyright  2010 Johannes Loehnert <loehnert.kde@gmx.de>
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

/***************************************************************************
 *   Copyright  2010 J. Loehnert <loehnert.kde@gmx.de>
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
#include "grid-voronoi.h"

#include <cmath>
#include <QPainterPath>
#include <QDebug>
#include <KLocalizedString>
#include "utilities.h"

void generateIrregularGrid(GoldbergEngine *e, int piece_count) {
}
void generateVoronoiGrid(GoldbergEngine *e, const QList<QPointF> cell_centers) {
}
/*
void generateIrregularGrid(GoldbergEngine *e, int piece_count) {
}

bool checkForQVoronoi() {
    // check if qvoronoi executable is there
}

struct VoronoiVertex {
    QPointF position;
    QList<GBClassicPlugParams*> connected_borders;
};
    
struct VoronoiCell {
    QPointF center;
    QList<int> neighbours;
    QList<GBClassicPlugParams> borders;
};

void generateVoronoiGrid(GoldbergEngine *e, const QList<QPointF> cell_centers) {
    // prepare list of pieces to create
    QList<VoronoiCell> cells = QList<VoronoiCell>();
    for (int n=0; n<cell_centers.size(); n++) {
        VoronoiCell c = VoronoiCell();
        c.center = cell_centers[n];
        cells.append(c);
    }

    // convert the cell center list into ASCII
    // shellout to qvoronoi, and ask it to return voronoi vertices (p) and ridges (Fv) for possibly degenerate (Qz) input.
    // read list of voronoi vertices
    QList<VoronoiVertex> cell_corners = QList<VoronoiVertex>();

    // GENERATE BORDERS by reading ridge list

    int ridge_count;

    for (int n=0; n<ridge_count; n++) {
        // read ridge line
        // get vertice coordinates 1 and 2
        // if second endpoint is infinity (0), swap it with the first, so that the infty endpoint is always first
        // if one endpoint is infinity (0), calculate the voronoi vertex by intersecting with the image frame
        // crop OOB endpoints to frame

        // add the cell with lower id to the neighbour list of the cell with higher id

        // create a border for the ridge
        // collision-check that border against all borders already connected to both endpoints
        // after succesfull collision check, add border to both VVertices' "connected" list

        // now add the border to the cells
        

    }

    // CREATE PIECES

    for (int n=0; n<cells.size(); n++) {
        // create cell pixmap
        // calculate angle for p1()-center and p2()-center of each border 
        // (if a2 < a1, swap angles and mark border as reversed)
        // argsort the borders by mean angle (a1+a2) (few items - use a simple O(N^2) insertsort)
        // init path
        // iterate over the sorted borders
        // if a1 of next border != a2 of last border, there must be a frame segment inbetween - add it
        // render piece
         
        // RELATIONS: iterate neighbour list and add relations. 
    }
}
*/


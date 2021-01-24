/*
    SPDX-FileCopyrightText: 2010 Johannes Loehnert <loehnert.kde@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "grid.h"

#include <cmath>
#include <QPainterPath>
#include <QDebug>
#include "utilities.h"

struct HexCell {
    // the border at the left edge, upper one
    GBClassicPlugParams uleft;
    // the border at the left edge, lower one
    GBClassicPlugParams lleft;
    // the horizontal border, either at the top cell edge
    // or vertically in the middle (odd cell)
    GBClassicPlugParams horiz;

    // id of piece in cell (upper one for odd column)
    int id;
};

void HexMode::generateGrid(GoldbergEngine *e, int piece_count) const {
    const qreal ONE_SIXTH = 1/6.0;

    int collision_tries = 10 * e->m_plug_size * e->m_plug_size;
    if (collision_tries < 5) collision_tries = 5;
    const qreal collision_shrink_factor = 0.95;

    int next_piece_id = 0;

    // calculate piece counts
    const int width = e->get_image_width(), height = e->get_image_height();
    int xCount;
    int yCount;
    getBestFitExtended(xCount, yCount, 1.0*width / height * 1.7320508075689 / 1.5, piece_count, 1.0, 0., 0.5, 0.);

    qDebug() << "cell count x = " << xCount;
    qDebug() << "cell count y = " << yCount;


    const qreal cellWidth = 1.0 * width / xCount, cellHeight = 1.0 * height / yCount;

    // rationale: knobs should visually cover the same fraction of area as for the rect grid.
    e->m_length_base = sqrt(cellWidth * cellHeight) * e->m_plug_size;

    // generate borders
    // grid is made 1 unit larger in both dimensions, to store the right and bottom borders.
    HexCell** cells = new HexCell*[xCount + 1];

    qDebug() << "now generating edges";

    for (int x = 0; x < xCount+1; ++x) {
        cells[x] = new HexCell[yCount+1];

        for (int y = 0; y < yCount+1; ++y) {
            bool odd_column = x%2;
            // generate usual borders first, and cater for the "edge" cases afterwards.
            cells[x][y].uleft = e->initEdge(false);
            cells[x][y].lleft = e->initEdge(false);
            cells[x][y].horiz = e->initEdge(false);

            // determine border direction
            cells[x][y].horiz.flipped ^= !e->m_alternate_flip;
            cells[x][y].uleft.flipped ^= (!odd_column);
            cells[x][y].lleft.flipped ^= odd_column ^ e->m_alternate_flip;

            if (e->m_alternate_flip && (y%2)) {
                cells[x][y].horiz.flipped ^= (!odd_column);
                cells[x][y].uleft.flipped ^= true;
                cells[x][y].lleft.flipped ^= true;
            }

            // determine border vectors
            qreal xleft1, xleft2, xright;
            xleft1 = odd_column? ((x-ONE_SIXTH) * cellWidth) : ((x + ONE_SIXTH) * cellWidth);
            xleft2 = odd_column? ((x+ONE_SIXTH) * cellWidth) : ((x - ONE_SIXTH) * cellWidth);
            xright = (x+1-ONE_SIXTH) * cellWidth;
            if (x==0 || x == xCount) {
                xleft1 = x * cellWidth;
                xleft2 = x * cellWidth;
            }
            if (x == xCount - 1) xright = (x+1)*cellWidth;

            // and set
            cells[x][y].uleft.unit_x = QLineF(xleft1, y * cellHeight, xleft2, (y + 0.5) * cellHeight);
            cells[x][y].lleft.unit_x = QLineF(xleft2, (y + 0.5) * cellHeight, xleft1, (y + 1.0) * cellHeight);
            cells[x][y].horiz.unit_x = QLineF(odd_column?xleft2:xleft1, (y + (odd_column?0.5:0.0)) * cellHeight, 
                                              xright, (y + (odd_column?0.5:0.0)) * cellHeight);

            // frame borders
            if (x==0 || x == xCount) {
                cells[x][y].uleft.is_straight = true;
                cells[x][y].lleft.is_straight = true;
            }
            if ((y==0 || y == yCount) && !odd_column) {
                cells[x][y].horiz.is_straight = true;
            }

            // collision checking
            // don't bother with the "outer" cells, they do not matter.
            if (x < xCount && y < yCount) {
                bool intersects = true;
                QList<GBClassicPlugParams*> offenders;
                // ULEFT
                for (int i=0; i<collision_tries && intersects; i++) {
                    offenders.clear();
                    if (i>0 && intersects) {
                        //qDebug() << "collision: uleft edge, x=" << x << ", y=" << y;
                        cells[x][y].uleft.size_correction *= collision_shrink_factor;
                        e->reRandomizeEdge(cells[x][y].uleft);
                    }
                    intersects = false;
                    if (x>0) intersects |= e->plugsIntersect(cells[x][y].uleft, cells[x-1][y].horiz, &offenders);
                    if (y>0) intersects |= e->plugsIntersect(cells[x][y].uleft, cells[x][y-1].lleft, &offenders);
                    if (y==0) intersects |= e->plugOutOfBounds(cells[x][y].uleft);
                }
                if (intersects) {
                    // give up and make the colliding borders plugless.
                    e->makePlugless(cells[x][y].uleft);
                    for (int i=0; i<offenders.size(); i++) e->makePlugless(*(offenders.at(i)));
                }

                // LLEFT
                intersects = true;
                for (int i=0; i<collision_tries && intersects; i++) {
                    offenders.clear();
                    if (i>0 && intersects) {
                        // qDebug() << "collision: lleft edge, x=" << x << ", y=" << y;
                        cells[x][y].lleft.size_correction *= collision_shrink_factor;
                        e->reRandomizeEdge(cells[x][y].lleft);
                    }
                    intersects = e->plugsIntersect(cells[x][y].lleft, cells[x][y].uleft, &offenders);
                    if (x!=0) { 
                        intersects |= (odd_column ? 
                                    e->plugsIntersect(cells[x][y].lleft, cells[x-1][y+1].horiz, &offenders) :
                                    e->plugsIntersect(cells[x][y].lleft, cells[x-1][y].horiz, &offenders)
                                );
                    }
                    if (y==yCount-1) intersects |= e->plugOutOfBounds(cells[x][y].lleft);
                }
                if (intersects) {
                    // give up and make the colliding borders plugless.
                    e->makePlugless(cells[x][y].lleft);
                    for (int i=0; i<offenders.size(); i++) e->makePlugless(*(offenders.at(i)));
                }

                // HORIZ
                intersects = true;
                for (int i=0; i<collision_tries && intersects; i++) {
                    offenders.clear();
                    if (i>0 && intersects) {
                        //qDebug() << "collision: horiz edge, x=" << x << ", y=" << y;
                        cells[x][y].horiz.size_correction *= collision_shrink_factor;
                        e->reRandomizeEdge(cells[x][y].horiz);
                    }
                    intersects = e->plugsIntersect(cells[x][y].horiz, cells[x][y].uleft, &offenders);
                    if (odd_column) {
                        intersects |= e->plugsIntersect(cells[x][y].horiz, cells[x][y].lleft, &offenders);
                    }
                    else {
                        if (y!=0) intersects |= e->plugsIntersect(cells[x][y].horiz, cells[x][y-1].lleft, &offenders);
                    }
                }
                if (intersects) {
                    // give up and make the colliding borders plugless.
                    e->makePlugless(cells[x][y].horiz);
                    for (int i=0; i<offenders.size(); i++) e->makePlugless(*(offenders.at(i)));
                }
            } // end collision checking
        }
    }

    // generate pieces
    qDebug() << "now creating pieces";


    for (int x = 0; x < xCount; ++x) {
        for (int y = 0; y < yCount+1; ++y) {
            QPainterPath path;
            bool odd_column = x%2;

            if (y==yCount && !odd_column) continue;

            path.moveTo(cells[x][y].uleft.unit_x.p1());
            if (!odd_column) {
                e->addPlugToPath(path, false, cells[x][y].horiz);
                e->addPlugToPath(path, false, cells[x+1][y].uleft);
                e->addPlugToPath(path, false, cells[x+1][y].lleft);
                e->addPlugToPath(path, true, cells[x][y+1].horiz);
                e->addPlugToPath(path, true, cells[x][y].lleft);
                e->addPlugToPath(path, true, cells[x][y].uleft);
            }
            else {
                // now we have to deal with the half pieces
                if (y==0) {
                    path.lineTo(cells[x+1][y].uleft.unit_x.p1());
                }
                else {
                    e->addPlugToPath(path, true, cells[x][y-1].lleft);
                    e->addPlugToPath(path, false, cells[x][y-1].horiz);
                    e->addPlugToPath(path, false, cells[x+1][y-1].lleft);
                }
                if (y==yCount) {
                    path.lineTo(cells[x][y].uleft.unit_x.p1());
                }
                else {
                    e->addPlugToPath(path, false, cells[x+1][y].uleft);
                    e->addPlugToPath(path, true, cells[x][y].horiz);
                    e->addPlugToPath(path, true, cells[x][y].uleft);
                }
            }

            cells[x][y].id = next_piece_id++;
            e->makePieceFromPath(cells[x][y].id, path);
        }
    }

    // generate relations
    qDebug() << "now adding relations";

    for (int x=0; x<xCount; x++) {
        for (int y=0; y<yCount+1; y++) {
            // piece above
            if (y>0 && (y<yCount + x%2)) e->addRelation(cells[x][y].id, cells[x][y-1].id);
            // piece to the right
            if (x>0 && y < yCount) e->addRelation(cells[x][y].id, cells[x-1][y].id);
            // other piece to the right
            if (x%2) {
                if (y>0) e->addRelation(cells[x][y].id, cells[x-1][y-1].id);
            }
            else {
                if (x>0 && y<yCount) e->addRelation(cells[x][y].id, cells[x-1][y+1].id);
            }
        }
    }

    qDebug() << "cleanup";
    // cleanup
    for (int x=0; x<xCount+1; x++) {
        delete[] cells[x];
    }
    delete[] cells;
}

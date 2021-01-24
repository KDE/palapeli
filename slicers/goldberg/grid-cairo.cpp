/*
    SPDX-FileCopyrightText: 2010 Johannes Loehnert <loehnert.kde@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "grid.h"

#include <cmath>
#include <QPainterPath>
#include <QDebug>
#include "utilities.h"

struct CairoCell {
    // corner : the border through the top left corner of the cell.
    GBClassicPlugParams corner;
    GBClassicPlugParams tl;
    GBClassicPlugParams tr;
    GBClassicPlugParams bl;
    GBClassicPlugParams br;

    // top and left tile of cell
    int id_top;
    int id_left;
};

void CairoMode::generateGrid(GoldbergEngine *e, int piece_count) const {
    int next_piece_id=0;

    int collision_tries = 10 * e->m_plug_size * e->m_plug_size;
    if (collision_tries < 5) collision_tries = 5;
    const qreal collision_shrink_factor = 0.95;


    // calculate piece counts
    const int width = e->get_image_width(), height = e->get_image_height();
    int xCount;
    int yCount;
    getBestFitExtended(xCount, yCount, 1.0 * width / height, piece_count, 2.0, 1., 1., 0.);

    qDebug() << "cell count x = " << xCount;
    qDebug() << "cell count y = " << yCount;


    const qreal cellWidth = 1.0 * width / xCount, cellHeight = 1.0 * height / yCount;

    // rationale: knobs should visually cover the same fraction of area as for the rect grid.
    e->m_length_base = sqrt(cellWidth * cellHeight * 0.5) * e->m_plug_size;

    // grid is made 1 unit larger in both dimensions, to store the right and bottom border cells.
    CairoCell** cells = new CairoCell*[xCount+1];

    qDebug() << "now generating edges";
 
    for (int x = 0; x < xCount+1; ++x) {
        cells[x] = new CairoCell[yCount + 1];

        for (int y = 0; y < yCount+1; ++y) {

            bool odd_cell = (x+y) % 2;

            // generate usual borders first, and cater for the "edge" cases afterwards.
            cells[x][y].corner = e->initEdge(false);
            cells[x][y].tr = e->initEdge(false);
            cells[x][y].tl = e->initEdge(false);
            cells[x][y].bl = e->initEdge(false);
            cells[x][y].br = e->initEdge(false);

            // determine border direction
            if (odd_cell) {
                cells[x][y].tr.flipped ^= true;
                cells[x][y].tl.flipped ^= true;
                cells[x][y].bl.flipped ^= true;
                cells[x][y].br.flipped ^= true;
            }

            cells[x][y].tr.flipped ^= e->m_alternate_flip;
            cells[x][y].br.flipped ^= e->m_alternate_flip;
            cells[x][y].bl.flipped ^= e->m_alternate_flip;
            cells[x][y].tl.flipped ^= e->m_alternate_flip;

            // set vector
            if (odd_cell) {
                cells[x][y].corner.flipped ^= (y%2 == 1);
                cells[x][y].corner.unit_x = QLineF((x-0.25) * cellWidth, y * cellHeight, (x+0.25) * cellWidth, y * cellHeight);
            }
            else {
                cells[x][y].corner.flipped ^= (x%2 == 1);
                cells[x][y].corner.unit_x = QLineF(x * cellWidth, (y-0.25) * cellHeight, x * cellWidth, (y + 0.25) * cellHeight);
            }

            cells[x][y].tl.unit_x = QLineF((x + (odd_cell?0.25:0.0 )) * cellWidth, 
                                           (y + (odd_cell?0.0 :0.25)) * cellHeight, (x+0.5) * cellWidth, (y+0.5) * cellHeight);
            cells[x][y].tr.unit_x = QLineF((x + (odd_cell?1.0 :0.75)) * cellWidth, 
                                           (y + (odd_cell?0.25:0.0 )) * cellHeight, (x+0.5) * cellWidth, (y+0.5) * cellHeight);
            cells[x][y].bl.unit_x = QLineF((x + (odd_cell?0.0 :0.25)) * cellWidth, 
                                           (y + (odd_cell?0.75:1.0 )) * cellHeight, (x+0.5) * cellWidth, (y+0.5) * cellHeight);
            cells[x][y].br.unit_x = QLineF((x + (odd_cell?0.75:1.0 )) * cellWidth, 
                                           (y + (odd_cell?1.0 :0.75)) * cellHeight, (x+0.5) * cellWidth, (y+0.5) * cellHeight);

            e->smooth_join(cells[x][y].tl, cells[x][y].br);
            e->smooth_join(cells[x][y].tr, cells[x][y].bl);

            // edges
            if (y==0) {
                if (!odd_cell) {
                    cells[x][y].corner.unit_x = QLineF(x * cellWidth, y * cellHeight, x * cellWidth, (y+ 0.25) * cellHeight);
                }
                else {
                    cells[x][y].corner.is_straight = true;
                }
            }
            if (x==0) {
                if (odd_cell) {
                    cells[x][y].corner.unit_x = QLineF(x * cellWidth, y * cellHeight, (x+0.25) * cellWidth, y * cellHeight);
                }
                else {
                    cells[x][y].corner.is_straight = true;
                }
            }

            if (y==yCount) {
                if (!odd_cell) {
                    cells[x][y].corner.unit_x = QLineF(x * cellWidth, (y-0.25) * cellHeight, x * cellWidth, y * cellHeight);
                }
                else {
                    cells[x][y].corner.is_straight = true;
                }
            }
            if (x==xCount) {
                if (odd_cell) {
                    cells[x][y].corner.unit_x = QLineF((x-0.25) * cellWidth, y * cellHeight, x * cellWidth, y * cellHeight);
                }
                else {
                    cells[x][y].corner.is_straight = true;
                }
            }

            // collision checking
            bool intersects;
            QList<GBClassicPlugParams*> offenders;

            // CORNER
            intersects = !cells[x][y].corner.is_straight;
            for (int i=0; i<collision_tries && intersects; i++) {
                offenders.clear();
                if (i>0 && intersects) {
                    //qDebug() << "collision: corner edge, x=" << x << ", y=" << y;
                    cells[x][y].corner.size_correction *= collision_shrink_factor;
                    e->reRandomizeEdge(cells[x][y].corner);
                }
                intersects = false;
                if (x>0 && y>0) intersects |= e->plugsIntersect(cells[x][y].corner, cells[x-1][y-1].br, &offenders);
                intersects |= (x==0) ? e->plugOutOfBounds(cells[x][y].corner) : e->plugsIntersect(cells[x][y].corner, cells[x-1][y].tr, &offenders);
                intersects |= (y==0) ? e->plugOutOfBounds(cells[x][y].corner) : e->plugsIntersect(cells[x][y].corner, cells[x][y-1].bl, &offenders);
                if (x==xCount || y==yCount) intersects |= e->plugOutOfBounds(cells[x][y].corner);
            }
            if (intersects) {
                // give up and just make the colliding borders plugless.
                e->makePlugless(cells[x][y].corner);
                for (int i=0; i<offenders.size(); i++) e->makePlugless(*(offenders.at(i)));
            }


            // TL
            // don't bother with the "outer" cells, they do not matter.
            intersects = (x<xCount && y < yCount);
            for (int i=0; i<collision_tries && intersects; i++) {
                offenders.clear();
                if (i>0 && intersects) {
                    //qDebug() << "collision: top left edge, x=" << x << ", y=" << y;
                    cells[x][y].tl.size_correction *= collision_shrink_factor;
                    e->reRandomizeEdge(cells[x][y].tl, true);
                }
                intersects = e->plugsIntersect(cells[x][y].tl, cells[x][y].corner, &offenders);
                intersects |= (odd_cell ?
                                ((y==0) ? e->plugOutOfBounds(cells[x][y].tl) : e->plugsIntersect(cells[x][y].tl, cells[x][y-1].bl, &offenders)) :
                                ((x==0) ? e->plugOutOfBounds(cells[x][y].tl) : e->plugsIntersect(cells[x][y].tl, cells[x-1][y].tr, &offenders))
                            );
            }
            if (intersects) {
                // give up and just make the colliding borders plugless.
                e->makePlugless(cells[x][y].tl);
                for (int i=0; i<offenders.size(); i++) e->makePlugless(*(offenders.at(i)));
            }

            // TR
            // don't bother with the "outer" cells, they do not matter.
            intersects = (x<xCount && y < yCount);
            for (int i=0; i<collision_tries && intersects; i++) {
                offenders.clear();
                if (i>0 && intersects) {
                    //qDebug() << "collision: top right edge, x=" << x << ", y=" << y;
                    cells[x][y].tr.size_correction *= collision_shrink_factor;
                    e->reRandomizeEdge(cells[x][y].tr, true);
                }
                intersects = e->plugsIntersect(cells[x][y].tr, cells[x][y].tl, &offenders);
                intersects |= (odd_cell ?
                                ((x==xCount-1) ? e->plugOutOfBounds(cells[x][y].tr) : false) :
                                ((y==0) ? e->plugOutOfBounds(cells[x][y].tr) : e->plugsIntersect(cells[x][y].tr, cells[x][y-1].br, &offenders))
                            );
            }
            if (intersects) {
                // give up and just make the colliding borders plugless.
                e->makePlugless(cells[x][y].tr);
                for (int i=0; i<offenders.size(); i++) e->makePlugless(*(offenders.at(i)));
            }

            // BL
            // don't bother with the "outer" cells, they do not matter.
            intersects = (x<xCount && y < yCount);
            for (int i=0; i<collision_tries && intersects; i++) {
                offenders.clear();
                if (i>0 && intersects) {
                    //qDebug() << "collision: bottom left edge, x=" << x << ", y=" << y;
                    cells[x][y].bl.size_correction *= collision_shrink_factor;
                    e->reRandomizeEdge(cells[x][y].bl, true);
                }
                intersects = e->plugsIntersect(cells[x][y].bl, cells[x][y].tl, &offenders);
                intersects |= (odd_cell ?
                                ((x==0) ? e->plugOutOfBounds(cells[x][y].bl) : e->plugsIntersect(cells[x][y].tr, cells[x-1][y].bl, &offenders)) :
                                ((y==yCount-1) ? e->plugOutOfBounds(cells[x][y].bl) : false)
                            );
            }
            if (intersects) {
                // give up and just make the colliding borders plugless.
                e->makePlugless(cells[x][y].bl);
                for (int i=0; i<offenders.size(); i++) e->makePlugless(*(offenders.at(i)));
            }

            // BR
            // don't bother with the "outer" cells, they do not matter.
            intersects = (x<xCount && y < yCount);
            for (int i=0; i<collision_tries && intersects; i++) {
                offenders.clear();
                if (i>0 && intersects) {
                    //qDebug() << "collision: bottom right edge, x=" << x << ", y=" << y;
                    cells[x][y].br.size_correction *= collision_shrink_factor;
                    e->reRandomizeEdge(cells[x][y].br, true);
                }
                intersects = e->plugsIntersect(cells[x][y].br, cells[x][y].tr, &offenders);
                intersects |= e->plugsIntersect(cells[x][y].br, cells[x][y].bl, &offenders);
                intersects |= (odd_cell ?
                                ((y==yCount-1) ? e->plugOutOfBounds(cells[x][y].br) : false) :
                                ((x==xCount-1) ? e->plugOutOfBounds(cells[x][y].br) : false)
                            );
            }
            if (intersects) {
                // give up and just make the colliding borders plugless.
                e->makePlugless(cells[x][y].br);
                for (int i=0; i<offenders.size(); i++) e->makePlugless(*(offenders.at(i)));
            }

        } // end collision checking
    }

    qDebug() << "now creating pieces";

    for (int x = 0; x < xCount+1; ++x) {
        // checkerboard pattern as above
        for (int y = 0; y < yCount+1; ++y) {
            //create the mask path
            QPainterPath path;
            
            bool odd_cell = (x+y) % 2;

            // we start after the "corner" edge.
            path.moveTo(cells[x][y].corner.unit_x.p2());

            // TOP PIECE
            if (x < xCount) {
                if (!odd_cell) e->addPlugToPath(path, true, cells[x][y].corner);
                if (y==0) {
                    // half piece
                    path.lineTo(cells[x+1][y].corner.unit_x.p1());
                }
                else {
                    e->addPlugToPath(path, false, cells[x][y-1].bl);
                    e->addPlugToPath(path, true, cells[x][y-1].br);
                }
                if (odd_cell) e->addPlugToPath(path, false, cells[x+1][y].corner);

                if (y==yCount) {
                    // half piece
                    path.lineTo(cells[x][y].corner.unit_x.p2());
                }
                else {
                    e->addPlugToPath(path, false, cells[x][y].tr);
                    e->addPlugToPath(path, true, cells[x][y].tl);
                }

                cells[x][y].id_top = next_piece_id++;
                e->makePieceFromPath(cells[x][y].id_top, path);
            }

            // LEFT PIECE
            if (y < yCount) {
                path = QPainterPath();
                path.moveTo(cells[x][y].corner.unit_x.p2());
                if (x==xCount) {
                    // half piece
                    path.lineTo(odd_cell ? cells[x-1][y].br.unit_x.p1() : cells[x][y+1].corner.unit_x.p2());
                }
                else {
                    e->addPlugToPath(path, false, cells[x][y].tl);
                    e->addPlugToPath(path, true, cells[x][y].bl);
                }
                if (!odd_cell) e->addPlugToPath(path, true, cells[x][y+1].corner);

                if (x==0) {
                    // half piece
                    path.lineTo(odd_cell ? cells[x][y].corner.unit_x.p1() : cells[x][y].tl.unit_x.p1());
                }
                else {
                    e->addPlugToPath(path, false, cells[x-1][y].br);
                    e->addPlugToPath(path, true, cells[x-1][y].tr);
                }

                if (odd_cell) e->addPlugToPath(path, false, cells[x][y].corner);
                
                cells[x][y].id_left = next_piece_id++;
                e->makePieceFromPath(cells[x][y].id_left, path);
            }
        }
    }

    qDebug() << "now adding relations";

    //create relations
    for (int x = 0; x < xCount+1; ++x) {
        for (int y = 0; y < yCount+1; ++y) {
            bool odd_cell = (x+y) % 2;
            // corner 
            if (odd_cell) {
                if (y>0 && y < yCount) e->addRelation(cells[x][y].id_left, cells[x][y-1].id_left);
            }
            else {
                if (x>0 && x < xCount) e->addRelation(cells[x][y].id_top, cells[x-1][y].id_top);
            }
            // inner-cell borders
            if (y < yCount && x < xCount) {
                // tl
                e->addRelation(cells[x][y].id_top, cells[x][y].id_left);
                // tr
                e->addRelation(cells[x][y].id_top, cells[x+1][y].id_left);
                // bl
                e->addRelation(cells[x][y].id_left, cells[x][y+1].id_top);
                //br
                e->addRelation(cells[x+1][y].id_left, cells[x][y+1].id_top);
            }
        }
    }

    //cleanup
    for (int x = 0; x < xCount+1; ++x) {
        delete[] cells[x];
    }
    delete[] cells;
}

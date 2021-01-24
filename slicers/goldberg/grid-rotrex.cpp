/*
    SPDX-FileCopyrightText: 2010 Johannes Loehnert <loehnert.kde@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "grid.h"

#include <cmath>
#include <QPainterPath>
#include <QDebug>
#include "utilities.h"


// see rotrex-grid.svg to understand everything.
struct RotrexCell {
    GBClassicPlugParams horiz;
    GBClassicPlugParams vert;
    GBClassicPlugParams tl;
    GBClassicPlugParams tr;
    GBClassicPlugParams bl;
    GBClassicPlugParams br;

    // piece ids...
    int id_corner; // of the piece centered in the top left cell corner (hex or rect)
    int id_tri;    // of the triangular piece on the top cell corner
    int id_rect;   // of the rect piece fully contained in the cell
};

void RotrexMode::generateGrid(GoldbergEngine *e, int piece_count) const {
    // fixed offsets in relative units of cell size. see image.
    // set as constants so we don't have to throw magic numbers around all the time.
    // square root of three - helper number
    const qreal _R3 = sqrt(3.0);
    const qreal _xunit = 1.5 + _R3;
    const qreal x1 = 0.5 / _xunit, x2 = 1.0 / _xunit, x3 = (0.5 + _R3) / _xunit, x4 = (1 + _R3) / _xunit;
    const qreal _yunit = 1.0 + 0.5*_R3;
    const qreal y1 = (0.5*_R3) / _yunit, y2 = 1.0 / _yunit;


    int next_piece_id = 0;

    int collision_tries = 10 * e->m_plug_size * e->m_plug_size;
    if (collision_tries < 5) collision_tries = 5;
    const qreal collision_shrink_factor = 0.95;

    // calculate piece counts
    const int width = e->get_image_width(), height = e->get_image_height();
    int xCount;
    int yCount;
    getBestFitExtended(xCount, yCount, 1.0*width / height * _yunit / _xunit, piece_count, 3.0, 1.0, 2.0, 1.0);

    qDebug() << "cell count x = " << xCount;
    qDebug() << "cell count y = " << yCount;

    const qreal cellWidth = 1.0 * width / xCount, cellHeight = 1.0 * height / yCount;


    e->m_length_base = sqrt(cellWidth * cellHeight / 3.0) * e->m_plug_size;

    // GENERATE BORDERS
    // grid is made 1 unit larger in both dimensions, to store the right and bottom borders.
    RotrexCell** cells = new RotrexCell*[xCount+1];

    for (int x=0; x<xCount+1; x++) {
        cells[x] = new RotrexCell[yCount+1];
        for (int y=0; y<yCount+1; y++) {
            bool odd_cell = (x+y) % 2;

            // generate usual borders first, and cater for the "edge" cases afterwards.
            cells[x][y].horiz = e->initEdge(false);
            cells[x][y].vert = e->initEdge(false);
            cells[x][y].tl = e->initEdge(false);
            cells[x][y].tr = e->initEdge(false);
            cells[x][y].bl = e->initEdge(false);
            cells[x][y].br = e->initEdge(false);

            // determine border direction
            cells[x][y].horiz.flipped ^= (!odd_cell) ^ e->m_alternate_flip;
            cells[x][y].vert.flipped ^= (!odd_cell) ^ e->m_alternate_flip;
            cells[x][y].tl.flipped ^= odd_cell ^ e->m_alternate_flip;
            cells[x][y].tr.flipped ^= odd_cell ^ e->m_alternate_flip;
            cells[x][y].bl.flipped ^= (!odd_cell) ^ e->m_alternate_flip;
            cells[x][y].br.flipped ^= (!odd_cell) ^ e->m_alternate_flip;

            // now for the mad sh**. Set edge vectors.
            cells[x][y].horiz.unit_x = QLineF(
                        (x - x1) * cellWidth,
                        (y + (odd_cell ? y2 : y1)) * cellHeight,
                        (x + x1) * cellWidth,
                        (y + (odd_cell ? y2 : y1)) * cellHeight);
            cells[x][y].vert.unit_x = QLineF(
                        (x + (odd_cell ? x1 : x4)) * cellWidth,
                        (y - y2) * cellHeight,
                        (x + (odd_cell ? x1 : x4)) * cellWidth,
                        (y + y2) * cellHeight);
            cells[x][y].tl.unit_x = QLineF(
                        (x + (odd_cell ? x3 : x2)) * cellWidth,
                        y * cellHeight,
                        (x + x1) * cellWidth,
                        (y + (odd_cell ? y2 : y1)) * cellHeight);
            cells[x][y].tr.unit_x = QLineF(
                        (x + (odd_cell ? x3 : x2)) * cellWidth,
                        y * cellHeight,
                        (x + x4) * cellWidth,
                        (y + (odd_cell ? y1 : y2)) * cellHeight);
            cells[x][y].bl.unit_x = QLineF(
                        (x + x1) * cellWidth,
                        (y + (odd_cell ? y2 : y1)) * cellHeight,
                        (x + (odd_cell ? x2 : x3)) * cellWidth,
                        (y + 1) * cellHeight);
            cells[x][y].br.unit_x = QLineF(
                        (x + x4) * cellWidth,
                        (y + (odd_cell ? y1 : y2)) * cellHeight,
                        (x + (odd_cell ? x2 : x3)) * cellWidth,
                        (y + 1) * cellHeight);

            // pieces at frame
            // top edge
            if (y==0) {
                cells[x][y].vert.unit_x.setP1(QPointF(
                            cells[x][y].vert.unit_x.x1(),
                            y * cellHeight));
            }
            // left edge
            if (x==0) {
                cells[x][y].horiz.unit_x.setP1(QPointF(
                            x * cellWidth,
                            cells[x][y].horiz.unit_x.y1()));
            }
            // right edge
            if (x==xCount) {
                cells[x][y].horiz.unit_x.setP2(QPointF(
                            x * cellWidth,
                            cells[x][y].horiz.unit_x.y2()));
            }

            // bottom edge
            if (y == yCount) {
                cells[x][y].vert.unit_x.setP2(QPointF(
                            cells[x][y].vert.unit_x.x2(),
                            y * cellHeight));
            }

            // COLLISION CHECKS
            bool intersects;
            QList<GBClassicPlugParams*> offenders;

            // horiz
            intersects = (y < yCount);
            for (int i=0; i<collision_tries && intersects; i++) {
                offenders.clear();
                if (i>0 && intersects) {
                    cells[x][y].horiz.size_correction *= collision_shrink_factor;
                    e->reRandomizeEdge(cells[x][y].horiz);
                }
                if (x==0) {
                    intersects = e->plugOutOfBounds(cells[x][y].horiz);
                }
                else {
                    if (!odd_cell) {
                        intersects = e->plugsIntersect(cells[x][y].horiz, cells[x-1][y].tr, &offenders)
                                    || e->plugsIntersect(cells[x][y].horiz, cells[x-1][y+1].vert, &offenders);
                    }
                    else {
                        intersects = e->plugsIntersect(cells[x][y].horiz, cells[x-1][y].br, &offenders)
                                    || e->plugsIntersect(cells[x][y].horiz, cells[x-1][y].vert, &offenders);
                    }
                }
                if (x==xCount) intersects |= e->plugOutOfBounds(cells[x][y].horiz);
            }
            if (intersects) {
                // give up and make colliding borders plugless.
                e->makePlugless(cells[x][y].horiz);
                for (int i=0; i<offenders.size(); i++) e->makePlugless(*(offenders.at(i)));
            }

            // vert
            intersects = (x < xCount);
            for (int i=0; i<collision_tries && intersects; i++) {
                offenders.clear();
                if (i>0 && intersects) {
                    cells[x][y].vert.size_correction *= collision_shrink_factor;
                    e->reRandomizeEdge(cells[x][y].vert);
                }
                intersects = false;
                if (y==0) {
                    intersects |= e->plugOutOfBounds(cells[x][y].vert);
                }
                else {
                    if (!odd_cell) {
                        intersects |= e->plugsIntersect(cells[x][y].vert, cells[x][y-1].br, &offenders);
                    }
                    else {
                        intersects |= e->plugsIntersect(cells[x][y].vert, cells[x][y-1].bl, &offenders);
                        intersects |= e->plugsIntersect(cells[x][y].vert, cells[x][y-1].horiz, &offenders);
                    }
                }
                if (odd_cell) intersects |= e->plugsIntersect(cells[x][y].vert, cells[x][y].horiz, &offenders);
                if (y==yCount) intersects |= e->plugOutOfBounds(cells[x][y].vert);
            }
            if (intersects) {
                // give up and make colliding borders plugless.
                e->makePlugless(cells[x][y].vert);
                for (int i=0; i<offenders.size(); i++) e->makePlugless(*(offenders.at(i)));
            }

            if (x<xCount && y<yCount) {
                // tl
                intersects = true;
                for (int i=0; i<collision_tries && intersects; i++) {
                    offenders.clear();
                    if (i>0 && intersects) {
                        cells[x][y].tl.size_correction *= collision_shrink_factor;
                        e->reRandomizeEdge(cells[x][y].tl);
                    }
                    intersects = (y>0 ? 
                            e->plugsIntersect(cells[x][y].tl, cells[x][y-1].bl, &offenders) :
                            e->plugOutOfBounds(cells[x][y].tl));
                    intersects |= (odd_cell ?
                            e->plugsIntersect(cells[x][y].tl, cells[x][y].vert, &offenders) :
                            e->plugsIntersect(cells[x][y].tl, cells[x][y].horiz, &offenders));
                }
                if (intersects) {
                    // give up and make colliding borders plugless.
                    e->makePlugless(cells[x][y].tl);
                    for (int i=0; i<offenders.size(); i++) e->makePlugless(*(offenders.at(i)));
                }

                // tr
                intersects = true;
                for (int i=0; i<collision_tries && intersects; i++) {
                    offenders.clear();
                    if (i>0 && intersects) {
                        cells[x][y].tr.size_correction *= collision_shrink_factor;
                        e->reRandomizeEdge(cells[x][y].tr);
                    }
                    intersects = (y>0 ?
                            e->plugsIntersect(cells[x][y].tr, cells[x][y-1].br, &offenders) :
                            e->plugOutOfBounds(cells[x][y].tr));
                    intersects |= e->plugsIntersect(cells[x][y].tr, cells[x][y].tl, &offenders);
                    if (!odd_cell) intersects |= e->plugsIntersect(cells[x][y].tr, cells[x][y].vert, &offenders);
                }
                if (intersects) {
                    // give up and make colliding borders plugless.
                    e->makePlugless(cells[x][y].tr);
                    for (int i=0; i<offenders.size(); i++) e->makePlugless(*(offenders.at(i)));
                }

                // bl
                intersects = true;
                for (int i=0; i<collision_tries && intersects; i++) {
                    offenders.clear();
                    if (i>0 && intersects) {
                        cells[x][y].bl.size_correction *= collision_shrink_factor;
                        e->reRandomizeEdge(cells[x][y].bl);
                    }
                    intersects = e->plugsIntersect(cells[x][y].bl, cells[x][y].tl, &offenders);
                    if (odd_cell) intersects |= e->plugsIntersect(cells[x][y].bl, cells[x][y].horiz, &offenders);
                    if (y==yCount-1) intersects |= e->plugOutOfBounds(cells[x][y].bl);
                }
                if (intersects) {
                    // give up and make colliding borders plugless.
                    e->makePlugless(cells[x][y].bl);
                    for (int i=0; i<offenders.size(); i++) e->makePlugless(*(offenders.at(i)));
                }

                // br
                intersects = true;
                for (int i=0; i<collision_tries && intersects; i++) {
                    offenders.clear();
                    if (i>0 && intersects) {
                        cells[x][y].br.size_correction *= collision_shrink_factor;
                        e->reRandomizeEdge(cells[x][y].br);
                    }
                    intersects = e->plugsIntersect(cells[x][y].br, cells[x][y].bl, &offenders);
                    intersects |= e->plugsIntersect(cells[x][y].br, cells[x][y].tr, &offenders);
                    if (y==yCount-1) intersects |= e->plugOutOfBounds(cells[x][y].br);
                }
                if (intersects) {
                    // give up and make colliding borders plugless.
                    e->makePlugless(cells[x][y].br);
                    for (int i=0; i<offenders.size(); i++) e->makePlugless(*(offenders.at(i)));
                }
            }

        }
    }


    // CREATE PIECES
    for (int x=0; x<xCount+1; x++) {
        for (int y=0; y<yCount+1; y++) {
            QPainterPath path;
            bool odd_cell = (x+y) % 2;

            // separate logic for even and odd cell (easier...)
            // the inner rect piece is common for both
            if (!odd_cell) {
                // corner piece is hexagonal.
                // this is a real beast since it might be halved or even quartered.
                path = QPainterPath();

                // upper half
                path.moveTo((x + (x==0 ? 0.0 : -x2)) * cellWidth, y * cellHeight);

                if (y==0) {
                    path.lineTo((x + (x==xCount ? 0.0 : x2)) * cellWidth, 0.0);
                }
                else {
                    if (x==0) {
                        path.lineTo(cells[x][y-1].horiz.unit_x.p1());
                    }
                    else {
                        e->addPlugToPath(path, true, cells[x-1][y-1].br);
                    }
                    e->addPlugToPath(path, false, cells[x][y-1].horiz);
                    if (x==xCount) {
                        path.lineTo(xCount * cellWidth, y * cellHeight);
                    }
                    else {
                        e->addPlugToPath(path, false, cells[x][y-1].bl);
                    }
                }
                // lower half
                if (y==yCount) {
                    path.lineTo((x + (x==0 ? 0.0 : -x2)) * cellWidth, y * cellHeight);
                }
                else {
                    if (x==xCount) {
                        path.lineTo(cells[x][y].horiz.unit_x.p2());
                    }
                    else {
                        e->addPlugToPath(path, false, cells[x][y].tl);
                    }
                    e->addPlugToPath(path, true, cells[x][y].horiz);
                    if (x==0) {
                        path.lineTo(x * cellWidth, y * cellHeight);
                    }
                    else {
                        e->addPlugToPath(path, true, cells[x-1][y].tr);
                    }
                }

                cells[x][y].id_corner = next_piece_id++;
                e->makePieceFromPath(cells[x][y].id_corner, path);

                // triangle piece
                if (x < xCount) {
                    path = QPainterPath();
                    path.moveTo(cells[x][y].tl.unit_x.p1());

                    if (y==0) {
                        path.lineTo((x+x4) * cellWidth, 0.0);
                    }
                    else {
                        e->addPlugToPath(path, true, cells[x][y-1].br);
                    }
                    e->addPlugToPath(path, false, cells[x][y].vert);
                    if (y==yCount) {
                        path.lineTo(cells[x][y].tl.unit_x.p1());
                    }
                    else {
                        e->addPlugToPath(path, true, cells[x][y].tr);
                    }

                    cells[x][y].id_tri = next_piece_id++;
                    e->makePieceFromPath(cells[x][y].id_tri, path);
                }

            }
            else {
                // rect piece
                // might be halved or quartered.
                path = QPainterPath();
                path.moveTo((x + (x==0 ? 0.0 : -x1)) * cellWidth, (y + (y==0 ? 0.0 : -y2)) * cellHeight);

                if (y==0) {
                    path.lineTo((x + (x==xCount ? 0.0 : x1)) * cellWidth, 0.0);
                }
                else {
                    e->addPlugToPath(path, false, cells[x][y-1].horiz);
                }
                if (x==xCount) {
                    path.lineTo(xCount * cellWidth, (y + (y==yCount ? 0.0 : y2)) * cellHeight);
                }
                else {
                    e->addPlugToPath(path, false, cells[x][y].vert);
                }
                if (y==yCount) {
                    path.lineTo((x + (x==0 ? 0 : -x1)) * cellWidth, yCount * cellHeight);
                }
                else {
                    e->addPlugToPath(path, true, cells[x][y].horiz);
                }
                if (x==0) {
                    path.lineTo(0.0, (y + (y==0 ? 0.0 : -y2)) * cellHeight);
                }
                else {
                    e->addPlugToPath(path, true, cells[x-1][y].vert);
                }

                cells[x][y].id_corner = next_piece_id++;
                e->makePieceFromPath(cells[x][y].id_corner, path);

                // trigonal piece
                if (x < xCount) {
                    path = QPainterPath();
                    path.moveTo(cells[x][y].tl.unit_x.p1());

                    if (y==yCount) {
                        path.lineTo((x+x1) * cellWidth, yCount * cellHeight);
                    }
                    else {
                        e->addPlugToPath(path, false, cells[x][y].tl);
                    }

                    e->addPlugToPath(path, true, cells[x][y].vert);

                    if (y==0) {
                        path.lineTo(cells[x][y].tl.unit_x.p1());
                    }
                    else {
                        e->addPlugToPath(path, false, cells[x][y-1].bl);
                    }
                    cells[x][y].id_tri = next_piece_id++;
                    e->makePieceFromPath(cells[x][y].id_tri, path);
                }
            }
            // inner rect piece
            if (x<xCount && y<yCount) {
                path = QPainterPath();
                path.moveTo(cells[x][y].tr.unit_x.p1());
                e->addPlugToPath(path, false, cells[x][y].tr);
                e->addPlugToPath(path, false, cells[x][y].br);
                e->addPlugToPath(path, true, cells[x][y].bl);
                e->addPlugToPath(path, true, cells[x][y].tl);

                cells[x][y].id_rect = next_piece_id++;
                e->makePieceFromPath(cells[x][y].id_rect, path);
            }
        }
    }

    // RELATIONS
    for (int x=0; x<xCount+1; x++) {
        for (int y=0; y<yCount+1; y++) {
            bool odd_cell = (x+y) % 2;

            // Each cell takes care of the relations corresponding to the borders it contains.
            // horiz
            if (y<yCount) e->addRelation(cells[x][y].id_corner, cells[x][y+1].id_corner);
            // vert
            if (x<xCount) e->addRelation(cells[x][y].id_tri, cells[x + (odd_cell ? 0 : 1)][y].id_corner);

            if (x<xCount && y<yCount) {
                e->addRelation(cells[x][y].id_tri, cells[x][y].id_rect);
                e->addRelation(cells[x][y].id_rect, cells[x][y+1].id_tri);
                e->addRelation(cells[x][y].id_rect, cells[x  ][y + (odd_cell ? 1 : 0)].id_corner);
                e->addRelation(cells[x][y].id_rect, cells[x+1][y + (odd_cell ? 0 : 1)].id_corner);
            }
        }
    }

    // CLEANUP
    for (int x=0; x<xCount+1; x++) {
        delete[] cells[x];
    }
    delete[] cells;

}

/*
    SPDX-FileCopyrightText: 2010 Johannes Loehnert <loehnert.kde@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "grid.h"

#include <cmath>
#include <QPainterPath>
#include <QDebug>
#include "utilities.h"

void RectMode::generateGrid(GoldbergEngine *e, int piece_count) const {
    // number of tries to resolve collision
    int collision_tries = 10 * e->m_plug_size * e->m_plug_size;
    if (collision_tries < 5) collision_tries = 5;
    const qreal collision_shrink_factor = 0.95;


    // calculate piece counts
    const int width = e->get_image_width(), height = e->get_image_height();

    int xCount;
    int yCount;
    getBestFit(xCount, yCount, 1.0 * width / height, piece_count);

    const int pieceWidth = width / xCount, pieceHeight = height / yCount;
    e->m_length_base = (pieceWidth + pieceHeight) * 0.5 * e->m_plug_size;

    GBClassicPlugParams** horizontalPlugParams = new GBClassicPlugParams*[xCount+1];
    GBClassicPlugParams** verticalPlugParams = new GBClassicPlugParams*[xCount+1];

    for (int x = 0; x < xCount+1; ++x) {
        horizontalPlugParams[x] = new GBClassicPlugParams[yCount+1];
        verticalPlugParams[x] = new GBClassicPlugParams[yCount+1];
        for (int y = 0; y < yCount+1; ++y) {
            bool odd_tile = (x+y) % 2;
            //borders along X axis
            if (y==0 || y==yCount) {
                horizontalPlugParams[x][y] = e->initEdge(true);
            }
            else {
                horizontalPlugParams[x][y] = e->initEdge(false);
            }
            horizontalPlugParams[x][y].flipped ^= odd_tile ^ e->m_alternate_flip;
            horizontalPlugParams[x][y].unit_x = QLineF(x*pieceWidth, y*pieceHeight, (x+1)*pieceWidth, y*pieceHeight);
            if (x>0 && x < xCount) e->smooth_join(horizontalPlugParams[x][y], horizontalPlugParams[x-1][y]);
            
            //borders along Y axis
            if (x==0 || x==xCount) {
                verticalPlugParams[x][y] = e->initEdge(true);
            }
            else {
                verticalPlugParams[x][y] = e->initEdge(false);
            }
            verticalPlugParams[x][y].flipped ^= odd_tile;
            verticalPlugParams[x][y].unit_x = QLineF(x*pieceWidth, y*pieceHeight, x*pieceWidth, (y+1)*pieceHeight);
            if (y>0 && y < yCount) e->smooth_join(verticalPlugParams[x][y], verticalPlugParams[x][y-1]);

            // collision checking
            if (x > 0 && x < xCount) {
                bool v_intersects = true;
                QList<GBClassicPlugParams*> offenders;
                for (int i=0; i<collision_tries && v_intersects; i++) {
                    offenders.clear();
                    if (i>0 && v_intersects) {
                        //qDebug() << "collision: vertical edge, x=" << x << ", y=" << y;
                        verticalPlugParams[x][y].size_correction *= collision_shrink_factor;
                        e->reRandomizeEdge(verticalPlugParams[x][y], true);
                    }
                    v_intersects = e->plugsIntersect(verticalPlugParams[x][y], horizontalPlugParams[x-1][y], &offenders);
                    if (y<yCount) v_intersects |= e->plugsIntersect(verticalPlugParams[x][y], horizontalPlugParams[x-1][y+1], &offenders);
                }
                if (v_intersects) {
                    e->makePlugless(verticalPlugParams[x][y]);
                    for (int i=0; i<offenders.size(); i++) e->makePlugless(*(offenders.at(i)));
                }
            }
            if (y>0 && y < yCount) {
                bool h_intersects = true;
                QList<GBClassicPlugParams*> offenders;
                for (int i=0; i<collision_tries && h_intersects; i++) {
                    offenders.clear();
                    if (i>0 && h_intersects) {
                        //qDebug() << "collision: horizontal edge, x=" << x << " y=" << y;
                        horizontalPlugParams[x][y].size_correction *= collision_shrink_factor;
                        e->reRandomizeEdge(horizontalPlugParams[x][y], true);
                    }
                    h_intersects = e->plugsIntersect(horizontalPlugParams[x][y], verticalPlugParams[x][y-1], &offenders);
                    if (x<xCount) h_intersects |= e->plugsIntersect(horizontalPlugParams[x][y], verticalPlugParams[x][y], &offenders);
                }
                if (h_intersects) {
                    e->makePlugless(horizontalPlugParams[x][y]);
                    for (int i=0; i<offenders.size(); i++) e->makePlugless(*(offenders.at(i)));
                }
            }
        }
    }

    //create pieces
    for (int x = 0; x < xCount; ++x) {
        for (int y = 0; y < yCount; ++y) {
            //create the mask path
            QPainterPath path;
            path.moveTo(horizontalPlugParams[x][y].unit_x.p1());

            // top, right, bottom, left plug
            e->addPlugToPath(path, false, horizontalPlugParams[x][y]);
            e->addPlugToPath(path, false, verticalPlugParams[x+1][y]);
            e->addPlugToPath(path, true, horizontalPlugParams[x][y+1]);
            e->addPlugToPath(path, true, verticalPlugParams[x][y]);

            e->makePieceFromPath(x + y * xCount, path);
        }
    }

    //create relations
    for (int x = 0; x < xCount; ++x) {
        for (int y = 0; y < yCount; ++y) {
            if (x != 0) e->addRelation(x + y * xCount, (x - 1) + y * xCount);
            if (y != 0) e->addRelation(x + y * xCount, x + (y - 1) * xCount);
        }
    }

    //cleanup
    for (int x = 0; x < xCount + 1; ++x) {
        delete[] horizontalPlugParams[x];
        delete[] verticalPlugParams[x];
    }
    delete[] horizontalPlugParams;
    delete[] verticalPlugParams;
}


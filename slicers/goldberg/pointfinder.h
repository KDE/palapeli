/*
    SPDX-FileCopyrightText: 2010 Johannes Loehnert <loehnert.kde@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef POINTFINDER_H
#define POINTFINDER_H

#include <QPointF>
#include <QList>

class PointFinder {
    public:
        PointFinder(int width, int height, qreal radius);
        ~PointFinder();
        void append(QPointF point);

        QList<QPointF> points();
        QList<QPointF> find_neighbours(QPointF point);
    protected:
        QList<QPointF> **m_boxes;
        QList<QPointF> m_points;
        int m_radius;
        int m_xbins;
        int m_ybins;
        int m_width;
        int m_height;
};

#endif

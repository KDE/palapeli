/*
    SPDX-FileCopyrightText: 2010 Johannes Loehnert <loehnert.kde@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "pointfinder.h"

#include <QLineF>

PointFinder::PointFinder(int width, int height, qreal radius) {
    m_height = height;
    m_width = width;
    m_radius = radius;
    m_xbins = int(m_width / m_radius) + 1;
    m_ybins = int(m_height / m_radius) + 1;

    m_boxes = new QList<QPointF>* [m_xbins];
    for (int nx=0; nx < m_xbins; nx++) m_boxes[nx] = new QList<QPointF> [m_ybins];


}

PointFinder::~PointFinder() {
    for (int nx=0; nx < m_xbins; nx++) delete[] m_boxes[nx];
    delete[] m_boxes;
}

void PointFinder::append(QPointF point) {
    int nx = point.x() / m_radius;
    int ny = point.y() / m_radius;
    m_points.append(point);
    if (nx >= 0 && ny >= 0 && nx < m_xbins && ny < m_ybins) {
        m_boxes[nx][ny].append(point);
    }
}

QList<QPointF> PointFinder::points() {
    return m_points;
}

QList<QPointF> PointFinder::find_neighbours(QPointF point) {
    QList<QPointF> result;
    int nx = point.x() / m_radius;
    int ny = point.y() / m_radius;
    for (int nnx=nx-1; nnx <= nx+1; nnx++) {
        if (nnx < 0 || nnx >= m_xbins) continue;
        for (int nny = ny-1; nny <= ny+1; nny++) {
            if (nny < 0 || nny >= m_ybins) continue;
            for (int i=0; i<m_boxes[nnx][nny].size(); i++) {
                QPointF other = m_boxes[nnx][nny][i];
                if (QLineF(point, other).length() < m_radius &&
                            point != other) {
                    result.append(other);
                }
            }
        }
    }
    return result;
}

/*
    SPDX-FileCopyrightText: 2010 Johannes Loehnert <loehnert.kde@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "grid.h"

#include <cmath>
#include <QtMath>
#include <QDebug>
#include <QPainterPath>
#include <QProcess>
#include <QRandomGenerator>
#include "pointfinder.h"
#include "utilities.h"

// auxiliary functions for serializing / unserializing

/// serialize a list of floats into their space-separated ascii representation
QByteArray serializeLine(QList<qreal> input) {
    QStringList result;
    for (int i=0; i<input.size(); ++i) {
        result.append(QString::number(input[i]));
    }
    return result.join( QStringLiteral( " " )).toLatin1();
}

/// unserializes the first item of the input into a list of space-separated ints and removes it.
QList<int> popIntLine(QList<QByteArray> &input) {
    QList<int> result;
    if (input.size() == 0) return result;

    const QStringList parts = QString::fromUtf8(input.takeFirst()).split(QLatin1Char(' '), Qt::SkipEmptyParts);
    bool ok;

    for (int i=0; i<parts.size(); ++i) {
        int n = parts[i].toInt(&ok);
        if (ok) {
            result.append(n);
        }
        else {
            qDebug() << "Failure converting to integer:" << parts[i];
        }
    }
    return result;
}

/// unserializes the first item of the input into a list of space-separated floats and removes it.
QList<qreal> popFloatLine(QList<QByteArray> &input) {
    QList<qreal> result;
    if (input.size() == 0) return result;

    const QStringList parts = QString::fromUtf8(input.takeFirst()).split(QLatin1Char(' '), Qt::SkipEmptyParts);
    bool ok;

    for (int i=0; i<parts.size(); ++i) {
        qreal x = parts[i].toDouble(&ok);
        if (ok) {
            result.append(x);
        }
        else {
            qDebug() << "Failure converting to float:" << parts[i];
        }
    }
    return result;
}

/** if part of the line from p1 to p2 is out of bounds, shorten it  
so that it is totally contained in the image frame. returns false
if no part of the line is within the bounds.
*/
bool crop_endpoints_to_frame(QPointF *p1, QPointF *p2, int width, int height) {
    QRectF frame = QRectF(0.0, 0.0, width, height);
    QLineF ridge = QLineF(*p1, *p2);

    // first check if line is completely within frame.
    bool p1_contained = frame.contains(*p1);
    bool p2_contained = frame.contains(*p2);
    if (p1_contained && p2_contained) return true;

    // no it is not, cropping is necessary.
    // determine intersection points with frame
    QPointF new_p1, new_p2;
    int new_points_found = 0;

    for (int i=0; i<4; i++) {
        QLineF border;
        switch (i) {
            case 0: border = QLineF(0, 0, width, 0); break;
            case 1: border = QLineF(0, 0, 0, height); break;
            case 2: border = QLineF(width, 0, width, height); break;
            case 3: border = QLineF(0, height, width, height); break;
        }
        if (new_points_found == 0) {
            if (QLineF::BoundedIntersection == border.intersects(ridge, &new_p1)) {
                // if one point is inside, there will be only 1 intersection point.
                // But, if one point is on the frame, we might have found it. check that.
                new_points_found = 1;
                if (p1_contained || p2_contained) {
                    if (new_p1!=*p1 && new_p1!=*p2) {
                        break;
                    }
                    else {
                        // pretend that nothing happened
                        new_points_found = 0;
                    }
                }
            }
        }
        else {
            if (QLineF::BoundedIntersection == border.intersects(ridge, &new_p2)) {
                // We have to set new_points_found > 1, in order to get
                // endpoints cropped correctly in the case where both points are
                // outside the frame, but the ridge passes through the frame.
                // The lack of the next line was probably the cause of the
                // crashes mentioned below
                new_points_found = 2;
                // no need to search further
                break;
            }
        }
    }

    // border completely oob?
    if (new_points_found == 0) return false;

    if (new_points_found == 1) {
        // either p1 or p2 is contained. modify the uncontained point.
        if (p1_contained) {
            *p2 = new_p1; 
        }
        else {
            *p1 = new_p1;
        }
        // Sometimes there is a crash which probably comes from bad cropping.
        // Hoping to catch it with this.
        // Problem should be fixed now - SB
        qDebug() << "p1contained:" << p1_contained << " p1:" << *p1 << " p2:" << *p2;
    }
    else {
        // modify so that the direction of the line remains unchanged.
        qreal l1 = QLineF(*p1, new_p1).length();
        qreal l2 = QLineF(*p1, new_p2).length();
        if (l1 <= l2) {
            *p1 = new_p1;
            *p2 = new_p2;
        }
        else {
            *p1 = new_p2;
            *p2 = new_p1;
        }
    }

    return true;
}

void add_frame_segment(QPainterPath &path, QPointF from, QPointF to, int width, int height) {
    // find out on which segments of the frame the points lie.
    // 0 = top, 1 = right, 2 = bottom, 3 = left
    int seg_from = -1, seg_to = -1;

    if (from.y() == 0) seg_from = 0;
    if (from.x() == width) seg_from = 1;
    if (from.y() == height) seg_from = 2;
    if (from.x() == 0) seg_from = 3;

    if (to.y() == 0) seg_to = 0;
    if (to.x() == width) seg_to = 1;
    if (to.y() == height) seg_to = 2;
    if (to.x() == 0) seg_to = 3;

    if (seg_from == -1 || seg_to == -1) {
        qDebug() << "add_frame_segment: one of the points is not on the frame!";
        qDebug() << "from" << from << "to" << to;
    }

    while (seg_from != seg_to) {
        switch (seg_from) {
            case 0: path.lineTo(QPointF(width, 0)); break;
            case 1: path.lineTo(QPointF(width, height)); break;
            case 2: path.lineTo(QPointF(0, height)); break;
            case 3: path.lineTo(QPointF(0, 0)); break;
        }
        seg_from ++;
        if (seg_from>3) seg_from = 0;
    }
    path.lineTo(to);
}

// end auxiliary functions

void IrregularMode::generateGrid(GoldbergEngine *e, int piece_count) const {
    PointFinder *pfinder, *new_pfinder;
    int width = e->get_image_width();
    int height = e->get_image_height();
    int steps = e->m_irregular_relaxation_steps;
    qreal radius = 1.5 * sqrt(1.0 * width * height / piece_count);

    if (piece_count < 2) piece_count = 2;

    pfinder = new PointFinder(width, height, radius);
    auto *generator = QRandomGenerator::global();
    for (int i=0; i<piece_count; ++i) {
        qreal x = 0.000001 * qreal(generator->bounded(1000000)) * width;
        qreal y = 0.000001 * qreal(generator->bounded(1000000)) * height;
        pfinder->append(QPointF(x, y));
    }

    // If you just take random points, you will find that the voronoi cells
    // are of very different sizes. For a jigsaw puzzle, this is not so nice.
    // To make the cells roughly equal size, we let the points push each
    // other out of the way (much like rubber balls in a box). By doing more
    // or less steps, one arrives at more or less equal cell sizes.
    for (int step = 0; step < steps; step++) {
        qreal step_factor;
        // step_factor determines the size of the step. it is
        // chosen so that the cells move roughly the same distance
        // in each step. The initial steps are smaller since the
        // "forces" are higher in unrelaxed state.
        if (step>=19) {
            step_factor = 0.5;
        }
        else {
            step_factor = 1.0/(20-step);
        }

        new_pfinder = new PointFinder(width, height, radius);
        QList<QPointF> points = pfinder->points();
        for (int i=0; i < points.size(); i++) {
            qreal x = points[i].x(), y = points[i].y();
            QList<QPointF> others = pfinder->find_neighbours(points[i]);
            QPointF force = QPointF(0.0, 0.0);
            for (int j=0; j<others.size(); j++) {
                qreal dist = QLineF(points[i], others[j]).length();
                // at 0 distance, force is 1; it shrinks linearly until
                // it reaches zero at "radius" distance.
                force += (points[i] - others[j]) / dist * (1.0 - dist/radius);
            }
            // repulsive walls
            if (x < 0.5*radius) force += QPointF(1.0 - 2*x/radius, 0.0);
            if (x > width - 0.5*radius) force -= QPointF(1.0 - 2*(width-x)/radius, 0.0);
            if (y < 0.5*radius) force += QPointF(0.0, 1.0 - 2*y/radius);
            if (y > height - 0.5*radius) force -= QPointF(0.0, 1.0 - 2*(height-y)/radius);

            // 0.5 : newtons 3rd law (force acts on both partners)
            force *= 0.5 * radius * step_factor;
            x += force.x();
            y += force.y();
            if (x<0.0) x = 0.0;
            if (y<0.0) y = 0.0;
            if (x>width) x = width;
            if (y>height) y = height;
            new_pfinder->append(QPointF(x, y));
        }
        delete pfinder;
        pfinder = new_pfinder;
        new_pfinder = nullptr;
    }


    generateVoronoiGrid(e, pfinder->points());
    delete pfinder;
}


bool IrregularMode::checkForQVoronoi() {
    QProcess process;

    process.start(QStringLiteral("qvoronoi"), QStringList());
    process.waitForStarted();
    if (process.error() == QProcess::FailedToStart) {
        return false;
    }
    process.close();
    return true;

}

struct VoronoiVertex {
    QPointF position;
    QList<GBClassicPlugParams*> connected_borders;
};
    
struct VoronoiCell {
    QPointF center;
    QList<int> neighbours;
    QList<GBClassicPlugParams*> borders;
    QList<int> border_from;
    QList<int> border_to;
};

void IrregularMode::generateVoronoiGrid(GoldbergEngine *e, QList<QPointF> cell_centers) const {
    QList<VoronoiVertex> cell_corners;
    QList<VoronoiCell> cells;
    QList<GBClassicPlugParams*> borders;
    int width = e->get_image_width();
    int height = e->get_image_height();

    int collision_tries = 10 * e->m_plug_size * e->m_plug_size;
    if (collision_tries < 5) collision_tries = 5;
    const qreal collision_shrink_factor = 0.95;

    e->m_length_base = qSqrt(width * height / cell_centers.size());

    QList<int> int_line;

    // prepare list of pieces to create
    for (int n=0; n<cell_centers.size(); n++) {
        VoronoiCell c = VoronoiCell();
        c.center = cell_centers[n];
        cells.append(c);
    }

    // convert the cell center list into ASCII
    QByteArray qvoronoi_input;
    qvoronoi_input.append("2\n"); // dimension
    // append a large box so that all ridges we care about are bounded.
    // cell_centers is not used afterwards, so we can modify it.
    cell_centers.append(QPointF(-width, -height));
    cell_centers.append(QPointF(-width, 2 * height));
    cell_centers.append(QPointF(2 * width, -height));
    cell_centers.append(QPointF(2 * width, 2 * height));
    qvoronoi_input.append(QString::number(cell_centers.size()).toLatin1()).append("\n");
    for (int n=0; n<cell_centers.size(); n++) {
        QList<qreal> coords;
        coords.append(cell_centers[n].x());
        coords.append(cell_centers[n].y());
        qvoronoi_input.append(serializeLine(coords)).append("\n");
    }

    //qDebug() << "INPUT for qvoronoi: " << qvoronoi_input;

    // shellout to qvoronoi, and ask it to return voronoi vertices (p) and ridges (Fv) for possibly degenerate (Qz) input.
    QProcess process;
    process.start(QStringLiteral("qvoronoi"), QStringList() << QStringLiteral("Qz") << QStringLiteral("p") << QStringLiteral("Fv"));
    process.waitForStarted();
    process.write(qvoronoi_input);
    process.closeWriteChannel();
    process.waitForFinished();
    QByteArray qvoronoi_output = process.readAll();
    //qDebug() << "OUTPUT of qvoronoi: " << qvoronoi_output;
    QList<QByteArray> qvoronoi_output_lines = qvoronoi_output.split('\n');


    // read list of voronoi vertices
    // first line is the dimension again, which we already know
    popIntLine(qvoronoi_output_lines); 
    // get corner count
    int_line = popIntLine(qvoronoi_output_lines);
    if (int_line.size() == 0) return; // bad output
    int n_corners = int_line[0];

    // read them
    for (int n=0; n<n_corners; n++) {
        cell_corners.append(VoronoiVertex());
        QList<qreal> vertex_coords = popFloatLine(qvoronoi_output_lines);
        if (vertex_coords.size() < 2) return; // bad output
        cell_corners.last().position = QPointF(vertex_coords[0], vertex_coords[1]);
    }

    // GENERATE BORDERS by reading ridge list

    int_line = popIntLine(qvoronoi_output_lines);
    if (int_line.size() == 0) return; // bad output
    int ridge_count = int_line[0];

    for (int n=0; n<ridge_count; n++) {
        // read ridge line
        int_line = popIntLine(qvoronoi_output_lines);
        if (int_line.size() < 5) return; // bad output
        // elements: 0: always 4; 1, 2 cell ids; 3, 4: vertices
        int cell1 = int_line[1];
        int cell2 = int_line[2];
        int vert1 = int_line[3] - 1;
        int vert2 = int_line[4] - 1;

        if (vert1 == -1 || vert2 == -1) continue;

        // get vertice coordinates 1 and 2
        QPointF p1, p2;
        p1 = cell_corners[vert1].position;
        p2 = cell_corners[vert2].position;

        // crop OOB endpoints to frame
        //qDebug() << "before crop: " << p1 << p2;
        bool ridge_oob = !crop_endpoints_to_frame(&p1, &p2, width, height);
        //qDebug() << "after crop: " << p1 << p2;

        // Add the cell with lower id to the neighbour list of the cell with
        // higher id, but only if none of both belongs to the "safety box"
        // and only if the ridge is not out of bounds.
        if ( ! ridge_oob && cell1 < cells.size() && cell2 < cells.size()) {
            if (cell1 < cell2) {
                cells[cell2].neighbours.append(cell1);
            }
            else {
                cells[cell1].neighbours.append(cell2);
            }
        }

        GBClassicPlugParams *p_plug;

        if (!ridge_oob) {
            // create a border for the ridge
            GBClassicPlugParams plug = e->initEdge(false);
            plug.unit_x = QLineF(p1, p2);
            // if border is short, make it plugless...
            if (plug.unit_x.length() < e->m_length_base * 0.3) e->makePlugless(plug);
            // and if it is *very* short, make it straight.
            if (plug.unit_x.length() < e->m_length_base * 0.15) plug.is_straight = true;

            // collision-check that border against all borders already connected to both endpoints
            // but only if it is visible
            if (!ridge_oob) {
                bool intersects = true;
                QList<GBClassicPlugParams*> offenders;

                for (int i=0; i<collision_tries && intersects; i++) {
                    offenders.clear();
                    if (i>0 && intersects) {
                        plug.size_correction *= collision_shrink_factor;
                        e->reRandomizeEdge(plug, false);
                    }
                    intersects = false;
                    if (cell1 < cells.size()) {
                        for (int j=0; j<cells[cell1].borders.size(); j++) {
                            if (cells[cell1].borders[j] == NULL) {
                                intersects |= e->plugOutOfBounds(plug);
                            }
                            else {
                                intersects |= e->plugsIntersect(plug, *(cells[cell1].borders[j]), &offenders);
                            }
                        }
                    }
                    if (cell2 < cells.size()) {
                        for (int j=0; j<cells[cell2].borders.size(); j++) {
                            if (cells[cell2].borders[j] == NULL) {
                                intersects |= e->plugOutOfBounds(plug);
                            }
                            else {
                                intersects |= e->plugsIntersect(plug, *(cells[cell2].borders[j]), &offenders);
                            }
                        }
                    }
                }
                if (intersects) {
                    // make the colliding borders plugless
                    e->makePlugless(plug);
                    for (int i=0; i<offenders.size(); i++) e->makePlugless(*(offenders.at(i)));
                }
            }

            p_plug = new GBClassicPlugParams();
            *p_plug = plug;
            borders.append(p_plug);

            // add border to both VVertices' "connected" list
            cell_corners[vert1].connected_borders.append(p_plug);
            cell_corners[vert2].connected_borders.append(p_plug);
        }
        else {
            // invisible ridge: no border definition
            p_plug = nullptr;
        }

        // add the border to the cells
        if (cell1 < cells.size()) {
            cells[cell1].borders.append(p_plug);
            cells[cell1].border_from.append(vert1);
            cells[cell1].border_to.append(vert2);
        }
        
        if (cell2 < cells.size()) {
            cells[cell2].borders.append(p_plug);
            cells[cell2].border_from.append(vert1);
            cells[cell2].border_to.append(vert2);
        }
    }

    // CREATE PIECES

    for (int n=0; n<cells.size(); n++) {
        int first_plug = 0;
        while (first_plug < cells[n].borders.size() && cells[n].borders[first_plug] == NULL) first_plug++;

        if (first_plug >= cells[n].borders.size()) {
            //qDebug() << "piece" << n << "has no visible borders, skipping it";
            continue;
            // this will probably lead to problems :-( (missing piece)
        }

        GBClassicPlugParams *p_plug;
        p_plug = cells[n].borders[first_plug];

        // calculate angle for p1()-center and p2()-center of first border 
        QLineF l1 = QLineF(cells[n].center, p_plug->unit_x.p1());
        QLineF l2 = QLineF(cells[n].center, p_plug->unit_x.p2());
        qreal anglediff = l1.angleTo(l2);

        QList<int> order = QList<int>();
        QList<bool> reversed = QList<bool>();
        QList<QLineF> vectors = QList<QLineF>();

        // we want to add the borders clockwise!!
        if (!(anglediff >= 0 && anglediff <= 360)) qDebug() << "anglediff out of expected range:" << anglediff;
        order.append(first_plug);
        reversed.append(anglediff < 180);
        if (reversed.first()) {
            vectors.append(QLineF(cells[n].borders[first_plug]->unit_x.p2(), cells[n].borders[first_plug]->unit_x.p1()));
        }
        else {
            vectors.append(cells[n].borders[first_plug]->unit_x);
        }

        // argsort the borders by connecting border_from and border_to, and mark reversions
        // this is a dumb O(N2) sort, but we are talking about, say, 6 listitems here.
        //qDebug() << "sorting borders for cell " << n;

        int current_border = first_plug;
        int first_vertex = reversed.last() ? cells[n].border_to[current_border] : cells[n].border_from[current_border];
        int current_vertex = reversed.last() ? cells[n].border_from[current_border] : cells[n].border_to[current_border];
        while (current_vertex != first_vertex) {
            for (int i=0; i<cells[n].borders.size(); i++) {
                if (i == current_border) continue;
                if (cells[n].border_from[i] == current_vertex) {
                    // skip "invisible" borders
                    if (cells[n].borders[i] != NULL) {
                        order.append(i);
                        reversed.append(false);
                        vectors.append(cells[n].borders[i]->unit_x);
                    }
                    current_border = i;
                    current_vertex = cells[n].border_to[i];
                    break;
                }
                if (cells[n].border_to[i] == current_vertex) {
                    if (cells[n].borders[i] != NULL) {
                        order.append(i);
                        reversed.append(true);
                        vectors.append(QLineF(cells[n].borders[i]->unit_x.p2(), cells[n].borders[i]->unit_x.p1()));
                    }
                    current_border = i;
                    current_vertex = cells[n].border_from[i];
                    break;
                }
            }
        }

        //qDebug() << "drawing path";
        // init path
        QPainterPath path;
        path.moveTo(vectors[0].p1());

        // iterate over the sorted borders
        for (int i=0; i<order.size(); i++) {
            e->addPlugToPath(path, reversed[i], *cells[n].borders[order[i]]);
            //qDebug() << "add border from" << vectors[i].p1() << "to" << vectors[i].p2();

            // if startpoint of next border != endpoint of last border, there must be a frame segment inbetween - add it
            if (vectors[i].p2() != vectors[(i+1) % order.size()].p1()) {
                add_frame_segment(path, vectors[i].p2(), vectors[(i+1) % order.size()].p1(), width, height);
                //qDebug() << "add frame segment";
            }
        }
        // render piece
        e->makePieceFromPath(n, path);
         
        // RELATIONS: iterate neighbour list and add relations. 
        for (int i=0; i<cells[n].neighbours.size(); i++) {
            e->addRelation(n, cells[n].neighbours[i]);
        }
    }

    // cleanup
    for (int i = 0; i < borders.size(); i++) {
        if (borders[i]!=NULL) delete borders[i];
    }
}


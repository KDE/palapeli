/*
    SPDX-FileCopyrightText: 2010 Johannes Loehnert <loehnert.kde@gmx.net>
    Based on the Jigsaw slicer by:
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "goldberg-engine.h"

#include <cmath>
#include <QPainter>
#include <QDebug>
#include <QDir>
#include <QRandomGenerator>
#include "utilities.h"



GoldbergEngine::GoldbergEngine(Pala::SlicerJob *job) {
    m_dump_grid = false;
    m_job = job;
    // QImage uses memsharing, so this won't actually copy the img.
    m_image = m_job->image();
}


void GoldbergEngine::set_dump_grid(bool dump) {
    if (m_dump_grid) {
        delete m_grid_image;
    }
    m_dump_grid = dump;
    if (m_dump_grid) {
        m_grid_image = new QImage(m_job->image().width(), m_job->image().height(), QImage::Format_RGB32);
        m_grid_image->fill(QColor(Qt::white).rgb());
    }
}

bool GoldbergEngine::get_dump_grid() {
    return m_dump_grid;
}

int GoldbergEngine::get_image_width() {
    return m_image.width();
}

int GoldbergEngine::get_image_height() {
    return m_image.height();
}

void GoldbergEngine::dump_grid_image() {
    if (m_dump_grid) {
        QString path = QDir::home().filePath(QStringLiteral("goldberg-slicer-dump.png"));
        qDebug() << "Dumping grid image to" << path;
        m_grid_image->save(path, nullptr, -1);
        delete m_grid_image;
        m_dump_grid = false;
    }
}


GBClassicPlugParams GoldbergEngine::initEdge(bool is_straight) {
    GBClassicPlugParams r;
    r.size_correction = 1.0;
    r.flipped = (QRandomGenerator::global()->bounded(100) < m_flip_threshold);
    r.is_straight = is_straight;
    r.is_plugless = false;
    r.path_is_rendered = false;
    r.path = QPainterPath();

    if (is_straight) {
        // init the params to sensible values even when they are not needed
        // (some fool might reset is_straight)
        r.startangle=0;
        r.endangle=0;
        r.basepos=0.5;
        r.basewidth=0.1;
        r.knobsize = 0.2;
        r.knobangle = 25;
        r.knobtilt = 0;
    }
    else {
        reRandomizeEdge(r);
    }
    return r;
}


void GoldbergEngine::reRandomizeEdge(GBClassicPlugParams &r, bool keep_endangles) {

    if (!keep_endangles) {
        qreal skew = (m_edge_curviness)/100. * 1.5;
        r.startangle = nonuniform_rand(2, -35, m_sigma_curviness, skew);
        r.endangle = nonuniform_rand(2, -35, m_sigma_curviness, skew);
        r.baseroundness = -dsin(fmin(r.startangle, r.endangle));
        if (r.baseroundness < 0) r.baseroundness = 0;
    }

    r.basepos = nonuniform_rand(0.2, 0.8, m_sigma_basepos, 0);
    r.basewidth = nonuniform_rand(0.1, 0.17, m_sigma_plugs, 0); // scales with knobscale
    r.knobsize = nonuniform_rand(0.17, 0.23, m_sigma_plugs, 0); // scales with knobscale
    r.knobangle = nonuniform_rand(10., 30., m_sigma_plugs, 0);
    r.knobtilt = nonuniform_rand(-20., 20., m_sigma_plugs, 0);

    r.path_is_rendered = false;
    r.path = QPainterPath();
}

void GoldbergEngine::smooth_join(GBClassicPlugParams &border1, GBClassicPlugParams &border2) {
    bool found, b1end, b2end;
    found = false;
    if (border1.unit_x.p1() == border2.unit_x.p1()) {
        found=true; b1end = false; b2end = false;
    }
    if (border1.unit_x.p1() == border2.unit_x.p2()) {
        found=true; b1end = false; b2end = true;
    }
    if (border1.unit_x.p2() == border2.unit_x.p1()) {
        found=true; b1end = true; b2end = false;
    }
    if (border1.unit_x.p2() == border2.unit_x.p2()) {
        found=true; b1end = true; b2end = true;
    }

    if (!found) {
        // no common endpoint. don't do anything
        qDebug() << "slicer-goldberg.cpp : smooth_join: was asked to smooth between non-adjacent borders.";
        return;
    }

    b1end ^= border1.flipped;
    b2end ^= border2.flipped;

    qreal a1 = b1end ? border1.endangle : border1.startangle;
    qreal a2 = b2end ? border2.endangle : border2.startangle;

    if (b1end ^ b2end) {
        a1 = 0.5*(a1-a2);
        a2 = -a1;
    }
    else {
        a1 = 0.5*(a1+a2);
        a2 = a1;
    }

    if (b1end) border1.endangle = a1; else border1.startangle = a1;
    if (b2end) border2.endangle = a2; else border2.startangle = a2;

    border1.path_is_rendered = false;
    border1.path = QPainterPath();
    border2.path_is_rendered = false;
    border2.path = QPainterPath();

}


bool GoldbergEngine::plugsIntersect(GBClassicPlugParams &candidate, GBClassicPlugParams &other, QList<GBClassicPlugParams*> *offenders) {
    if (!candidate.path_is_rendered) renderClassicPlug(candidate);
    if (!other.path_is_rendered) renderClassicPlug(other);

    bool result = candidate.path.intersects(other.path);
    if (result && offenders!=nullptr) {
        offenders->append(&other);
    }
    return result;
}

bool GoldbergEngine::plugOutOfBounds(GBClassicPlugParams &candidate) {
    if (!candidate.path_is_rendered) renderClassicPlug(candidate);

    QPainterPath imagerect = QPainterPath(QPointF(0.0, 0.0));
    imagerect.lineTo(QPointF(m_image.width(), 0.0));
    imagerect.lineTo(QPointF(m_image.width(), m_image.height()));
    imagerect.lineTo(QPointF(0.0, m_image.height()));
    imagerect.closeSubpath();

    return (!imagerect.contains(candidate.path));
}

void GoldbergEngine::makePlugless(GBClassicPlugParams &parameters){
    parameters.is_plugless = true;
    parameters.size_correction = 1.0;
    parameters.path_is_rendered = false;
    parameters.path = QPainterPath();
}

void GoldbergEngine::makePieceFromPath(int piece_id, QPainterPath path) {

    path.closeSubpath();

    //determine the required size of the mask
    const QRect maskRect = path.boundingRect().toAlignedRect();
    //create the mask
    QImage mask(maskRect.size(), QImage::Format_ARGB32_Premultiplied);
    mask.fill(0x00000000); //fully transparent color
    QPainter painter(&mask);
    painter.translate(- maskRect.topLeft());
    if (m_outlines) {
        painter.setPen(Qt::NoPen);
    }
    else {
        painter.setPen(QPen(Qt::black, 1.0)); //we explicitly use a pen stroke in order to let the pieces overlap a bit (which reduces rendering glitches at the edges where puzzle pieces touch)
        // 1.0 still leaves the slightest trace of a glitch. but making the stroke thicker makes the plugs appear non-matching even when they belong together.
    }
    painter.setBrush(Qt::black);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawPath(path);
    painter.end();

    // create the piece (copied over from libpala)
    QPoint offset = maskRect.topLeft();
    QImage pieceImage(mask);
    QPainter piecePainter(&pieceImage);
    piecePainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    piecePainter.drawImage(QPoint(), safeQImageCopy(m_image, QRect(offset, mask.size())));

    // Outline -- code was left in though option was removed (rendering done by palapeli itself now)
    if (m_outlines) {
        piecePainter.translate(-offset);
        piecePainter.setRenderHint(QPainter::Antialiasing);
        piecePainter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        piecePainter.setBrush(Qt::NoBrush);

        QPen outlinePen = QPen();
        outlinePen.setWidth(m_length_base / 33.0);
        QColor opColor = QColor(0,0,0, 64);
        outlinePen.setColor(opColor);
        piecePainter.setPen(outlinePen);
        piecePainter.drawPath(path);

    }
    piecePainter.end();

    m_job->addPiece(piece_id, pieceImage, maskRect.topLeft());
}

//A modified version of QImage::copy, which avoids rendering errors even if rect is outside the bounds of the source image.
QImage safeQImageCopy(const QImage& source, const QRect& rect)
{
	QRect targetRect(QPoint(), rect.size());
	//copy image
	QImage target(rect.size(), source.format());
	QPainter p(&target);
	p.drawImage(targetRect, source, rect);
	p.end();
	return target;
	//Strangely, source.copy(rect) does not work. It produces black borders.
}

void GoldbergEngine::addRelation(int piece1, int piece2) {
    m_job->addRelation(piece1, piece2);
}


void GoldbergEngine::addPlugToPath(QPainterPath& path, bool reverse, GBClassicPlugParams &params) {

    if (!params.path_is_rendered) renderClassicPlug(params);

    if (!reverse) {
        path.connectPath(params.path);

        if (m_dump_grid) {
            // The idea here is that each border is drawn exactly twice - once forward, once reversed.
            // So if we catch the forward case, we will draw all borders once.
            QPainter borderPainter(m_grid_image);
            QPen outlinePen = QPen();
            outlinePen.setWidth(m_length_base / 50.0);
            outlinePen.setColor(QColor(Qt::black));

            borderPainter.setPen(outlinePen);
            borderPainter.setRenderHint(QPainter::Antialiasing);
            borderPainter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            borderPainter.setBrush(Qt::NoBrush);
            borderPainter.drawPath(path);
        }
    }
    else {
        path.connectPath(params.path.toReversed());
    }
}

void GoldbergEngine::renderClassicPlug(GBClassicPlugParams &params) {
    // unit_x gives offset and direction of the x base vector. Start and end should be the grid points.

    params.path_is_rendered = true;

    // move the endpoints inwards an unnoticable bit, so that the intersection detector
    // won't trip on the common endpoint.
    QLineF u_x = QLineF(params.unit_x.pointAt(0.0001), params.unit_x.pointAt(0.9999));
    //QLineF u_x = params.unit_x;

    params.path.moveTo(u_x.p1());

    if (params.is_straight) {
        params.path.lineTo(u_x.p2());
        return;
    }
    if (params.flipped) {
        u_x = QLineF(u_x.p2(), u_x.p1());
    }


    QLineF u_y = u_x.normalVector();
    // move y unit to start at (0,0).
    u_y.translate(-u_y.p1());

    qreal scaling = m_length_base / u_x.length() * params.size_correction;
    if (params.basewidth * scaling > 0.8) {
        // Plug is too large for the edge length. Make it smaller.
        scaling = 0.8 / params.basewidth;
        qDebug() << "shrinking a plug";
    }

    // some magic numbers here... carefully fine-tuned, better leave them as they are.
    qreal ends_ctldist = 0.4;
    //qreal base_lcdist = 0.1 * scaling;
    qreal base_ucdist = 0.05 * scaling;
    qreal knob_lcdist = 0.6 * params.knobsize * scaling;
    qreal knob_ucdist = 0.8 * params.knobsize * scaling;

    // set up piece -- here is where the really interesting stuff happens.
    // We will work from the ends inwards, so that symmetry counterparts are adjacent.
    // The QLine.pointAt function is used to transform everything into the coordinate
    // space defined by the us.
    // -- end points

    qreal r1y = ends_ctldist * params.basepos * dsin(params.startangle);
    qreal q6y = ends_ctldist * (1.-params.basepos) * dsin(params.endangle);
    QPointF p1 = u_x.p1();
    QPointF p6 = u_x.p2();
    QPointF r1 = u_x.pointAt(ends_ctldist * params.basepos * dcos(params.startangle)) +
                 u_y.pointAt(r1y);
    QPointF q6 = u_x.pointAt(1. - ends_ctldist * (1.-params.basepos) * dcos(params.endangle)) +
                 u_y.pointAt(q6y);

    // -- base points
    qreal p2x = params.basepos - 0.5 * params.basewidth * scaling;
    qreal p5x = params.basepos + 0.5 * params.basewidth * scaling;

    if (p2x < 0.1 || p5x > 0.9) {
        // knob to large. center knob on the edge. (params.basewidth * scaling < 0.8 -- see above)
        p2x = 0.5 - 0.5 * params.basewidth * scaling;
        p5x = 0.5 + 0.5 * params.basewidth * scaling;
    }

    //qreal base_y = r1y > q6y ? r1y : q6y;
    //qreal base_y = 0.5*(r1y + q6y);
    qreal base_y = -params.baseroundness * ends_ctldist * fmin(p2x, 1.-p5x);
    if (base_y > 0) base_y = 0;

    qreal base_lcy = base_y * 2.0;

    base_y += base_ucdist/2;
    base_lcy -= base_ucdist/2;
    //qreal base_lcy = r1y;
    //if (q6y < r1y) base_lcy = q6y;

    // at least -base_ucdist from base_y
    //if (base_lcy > base_y - base_ucdist) base_lcy = base_y-base_ucdist;

    QPointF q2 = u_x.pointAt(p2x) + 
                 u_y.pointAt(base_lcy);
    QPointF r5 = u_x.pointAt(p5x) +
                 u_y.pointAt(base_lcy);
    QPointF p2 = u_x.pointAt(p2x) +
                 u_y.pointAt(base_y);
    QPointF p5 = u_x.pointAt(p5x) +
                 u_y.pointAt(base_y);
    QPointF r2 = u_x.pointAt(p2x) +
                 u_y.pointAt(base_y + base_ucdist);
    QPointF q5 = u_x.pointAt(p5x) +
                 u_y.pointAt(base_y + base_ucdist);

    if (params.is_plugless) {
        if (!params.flipped) {
            params.path.cubicTo(r1, q2, p2);
            params.path.cubicTo(r2, q5, p5);
            params.path.cubicTo(r5, q6, p6);
        }
        else {
            params.path.cubicTo(q6, r5, p5);
            params.path.cubicTo(q5, r2, p2);
            params.path.cubicTo(q2, r1, p1);
        }
        return;
    }

    // -- knob points
    qreal p3x = p2x - params.knobsize * scaling * dsin(params.knobangle - params.knobtilt);
    qreal p4x = p5x + params.knobsize * scaling * dsin(params.knobangle + params.knobtilt);
    // for the y coordinate, knobtilt sign was swapped. Knobs look better this way...
    // like x offset from base points y, but that is 0.
    qreal p3y = params.knobsize * scaling * dcos(params.knobangle + params.knobtilt) + base_y;
    qreal p4y = params.knobsize * scaling * dcos(params.knobangle - params.knobtilt) + base_y;

    QPointF q3 = u_x.pointAt(p3x) +
                 u_y.pointAt(p3y - knob_lcdist);
    QPointF r4 = u_x.pointAt(p4x) +
                 u_y.pointAt(p4y - knob_lcdist);
    QPointF p3 = u_x.pointAt(p3x) +
                 u_y.pointAt(p3y);
    QPointF p4 = u_x.pointAt(p4x) +
                 u_y.pointAt(p4y);
    QPointF r3 = u_x.pointAt(p3x) +
                 u_y.pointAt(p3y + knob_ucdist);
    QPointF q4 = u_x.pointAt(p4x) +
                 u_y.pointAt(p4y + knob_ucdist);

    // done setting up. construct path.
    // if flipped, add points in reverse.
    if (!params.flipped) {
        params.path.cubicTo(r1, q2, p2);
        params.path.cubicTo(r2, q3, p3);
        params.path.cubicTo(r3, q4, p4);
        params.path.cubicTo(r4, q5, p5);
        params.path.cubicTo(r5, q6, p6);
    }
    else {
        params.path.cubicTo(q6, r5, p5);
        params.path.cubicTo(q5, r4, p4);
        params.path.cubicTo(q4, r3, p3);
        params.path.cubicTo(q3, r2, p2);
        params.path.cubicTo(q2, r1, p1);
    }
}


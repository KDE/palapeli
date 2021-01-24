/*
    SPDX-FileCopyrightText: 2010 Johannes Loehnert <loehnert.kde@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELISLICERS_GOLDBERG_ENGINE_H
#define PALAPELISLICERS_GOLDBERG_ENGINE_H

#include <Pala/SlicerJob>
#include <QPainterPath>

struct GBClassicPlugParams {
    bool flipped;
    bool is_plugless;
    bool is_straight;
    QLineF unit_x;
    qreal size_correction;

    // the rendered segment, so it only has to be rendered once.
    QPainterPath path;
    bool path_is_rendered;

    // start, end-angle: angle of ctl point at first and last node
    qreal startangle, endangle;
    // base roundness: how "curvy" the baseline is. 0..1.
    qreal baseroundness;
    // basepos, basewidth: x-center and distance of base points
    qreal basepos, basewidth;
    // knobsize: distance of knob ctl points from base points
    // knobangle, knobtilt: hard to describe.. they determine width
    // and asymmetry of the knob.
    qreal knobsize, knobangle, knobtilt;

};


class GoldbergEngine {
    public:
        int m_quickpreset;
        int m_flip_threshold;
        bool m_alternate_flip;
        int m_edge_curviness;
        qreal m_plug_size;
        qreal m_sigma_curviness;
        qreal m_sigma_basepos;
        qreal m_sigma_plugs;
        int m_irregular_relaxation_steps;
        bool m_outlines;
        // length of a "normal" border. 
        // determines the actual size of the knob.
        qreal m_length_base;

        explicit GoldbergEngine(Pala::SlicerJob *job);

        void set_dump_grid(bool dump);
        bool get_dump_grid();

        int get_image_width();
        int get_image_height();

        GBClassicPlugParams initEdge(bool is_straight);
        void reRandomizeEdge(GBClassicPlugParams &params, bool keep_endangles=false);
        // changes ctl point angles at the common end point to be equal.
        // the common endpoint of border1 and border2 is autodetected.
        void smooth_join(GBClassicPlugParams &border1, GBClassicPlugParams &border2);

        void makePieceFromPath(int piece_id, QPainterPath path);
        void addRelation(int piece1, int piece2);
        // checks if candidate intersects with an already set border.
        // If *offenders is given and the plugs intersect, a reference to the "other" border is added to the list.
        // If any plug is not rendered yet, this is done beforehands.
        bool plugsIntersect(GBClassicPlugParams &candidate, GBClassicPlugParams &other, QList<GBClassicPlugParams*>* offenders = nullptr);
        // checks if candidate is out of bounds (image frame).
        // If candidate is not rendered yet, this is done beforehands.
        bool plugOutOfBounds(GBClassicPlugParams &candidate);
        void makePlugless(GBClassicPlugParams &parameters);

        void addPlugToPath(QPainterPath& path, bool reverse, GBClassicPlugParams &parameters);
        void renderClassicPlug(GBClassicPlugParams &parameters);

        void dump_grid_image();

    protected:
        Pala::SlicerJob *m_job;
        QImage m_image;
        bool m_dump_grid;
        QImage *m_grid_image;
};

#endif // PALAPELISLICERS_GOLDBERG_ENGINE_H

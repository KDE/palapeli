/*
    SPDX-FileCopyrightText: 2010 Johannes Loehnert <loehnert.kde@gmx.de>
    Based on the Jigsaw slicer by:
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELISLICERS_SLICER_GOLDBERG_H
#define PALAPELISLICERS_SLICER_GOLDBERG_H

#include <Pala/Slicer>
#include <Pala/SlicerJob>
#include <Pala/SlicerProperty>

class GoldbergSlicer : public Pala::Slicer {
    Q_OBJECT
    public:
        explicit GoldbergSlicer(QObject* parent = nullptr, const QVariantList& args = QVariantList());
        bool run(Pala::SlicerJob* job) override;
    private:
        bool m_qvoronoi_available;

};


#endif // PALAPELISLICERS_GOLDBERG_SLICER_H

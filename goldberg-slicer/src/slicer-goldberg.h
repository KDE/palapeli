/***************************************************************************
 *   Copyright  2010 J. Loehnert <loehnert.kde@gmx.de>
 * based on the Jigsaw slicer (c) 2009 Stefan Majewsky <majewsky@gmx.net>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef PALAPELISLICERS_GOLDBERG_SLICER_H
#define PALAPELISLICERS_GOLDBERG_SLICER_H

#include <Pala/Slicer>
#include <Pala/SlicerJob>
#include <Pala/SlicerProperty>
#include <Pala/SlicerPropertySet>

#include "goldberg-engine.h"

#include "grid-rect.h"
#include "grid-hex.h"
#include "grid-cairo.h"
#include "grid-rotrex.h"
#include "grid-voronoi.h"

class GoldbergSlicer : public Pala::Slicer {
    Q_OBJECT
    public:
        explicit GoldbergSlicer(QObject* parent = 0, const QVariantList& args = QVariantList());
        virtual bool run(Pala::SlicerJob* job);
    protected:
        QVariantList m_tesselations;

};


#endif // PALAPELISLICERS_GOLDBERG_SLICER_H

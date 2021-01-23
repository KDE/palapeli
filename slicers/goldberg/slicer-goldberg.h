/***************************************************************************
 *   Copyright  2010 Johannes Loehnert <loehnert.kde@gmx.de>
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

#ifndef PALAPELISLICERS_SLICER_GOLDBERG_H
#define PALAPELISLICERS_SLICER_GOLDBERG_H

#include "../../libpala/slicer.h"
#include "../../libpala/slicerjob.h"
#include "../../libpala/slicerproperty.h"

class GoldbergSlicer : public Pala::Slicer {
    Q_OBJECT
    public:
        explicit GoldbergSlicer(QObject* parent = nullptr, const QVariantList& args = QVariantList());
        bool run(Pala::SlicerJob* job) Q_DECL_OVERRIDE;
    private:
        bool m_qvoronoi_available;

};


#endif // PALAPELISLICERS_GOLDBERG_SLICER_H

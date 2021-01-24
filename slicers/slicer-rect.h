/*
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELISLICERS_RECTSLICER_H //krazy:excludeall=includes
#define PALAPELISLICERS_RECTSLICER_H

#include "../libpala/slicer.h"
#include "../libpala/slicerjob.h"
#include "../libpala/slicerproperty.h"
#include "../libpala/slicerpropertyset.h"

class RectSlicer : public Pala::Slicer, public Pala::SimpleGridPropertySet
{
	Q_OBJECT
	public:
		explicit RectSlicer(QObject* parent = nullptr, const QVariantList& args = QVariantList());
		bool run(Pala::SlicerJob* job) override;
};

#endif // PALAPELISLICERS_RECTSLICER_H

/*
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELISLICERS_JIGSAW_SLICER_H //krazy:excludeall=includes
#define PALAPELISLICERS_JIGSAW_SLICER_H

#include <Pala/Slicer>
#include <Pala/SlicerJob>
#include <Pala/SlicerProperty>
#include <Pala/SlicerPropertySet>

struct JigsawPlugParams
{
	qreal plugPosition, plugLength, plugWidth;
	qreal distortion1, distortion2;
	qreal baseHeight, baseDistortion;

	static JigsawPlugParams createRandomParams();
	JigsawPlugParams mirrored();
};

class JigsawSlicer : public Pala::Slicer, public Pala::SimpleGridPropertySet
{
	Q_OBJECT
	public:
		explicit JigsawSlicer(QObject* parent = nullptr, const QVariantList& args = QVariantList());
		bool run(Pala::SlicerJob* job) override;
	protected:
		void addPlugToPath(QPainterPath& path, qreal plugNormLength, const QLineF& line, const QPointF& plugDirection, const JigsawPlugParams& parameters);
};

#endif // PALAPELISLICERS_JIGSAW_SLICER_H

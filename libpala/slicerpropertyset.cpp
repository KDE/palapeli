/*
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "slicerpropertyset.h"
#include "slicer.h"
#include "slicerjob.h"
#include "slicerproperty.h"

#include <cmath>
#include "libpala_debug.h"
#include <KLocalizedString>

//BEGIN Private classes

class Pala::SlicerPropertySetPrivate
{
public:
	explicit SlicerPropertySetPrivate(Pala::Slicer* slicer)
	    : m_slicer(slicer)
	{
	}

	Pala::Slicer* const m_slicer;
};

class Pala::SimpleGridPropertySetPrivate : public Pala::SlicerPropertySetPrivate
{
public:
	explicit SimpleGridPropertySetPrivate(Pala::Slicer* slicer)
	    : SlicerPropertySetPrivate(slicer)
	{
	}
};

//END Private classes

//BEGIN Pala::SlicerPropertySet

Pala::SlicerPropertySet::SlicerPropertySet(Pala::Slicer* slicer)
	: SlicerPropertySet(*new Pala::SlicerPropertySetPrivate(slicer))
{
}

Pala::SlicerPropertySet::SlicerPropertySet(SlicerPropertySetPrivate& dd)
	: d_ptr(&dd)
{
}

Pala::SlicerPropertySet::~SlicerPropertySet() = default;

Pala::Slicer* Pala::SlicerPropertySet::slicer() const
{
	Q_D(const SlicerPropertySet);
	return d->m_slicer;
}

void Pala::SlicerPropertySet::addPropertyToSlicer(const QByteArray& key, Pala::SlicerProperty* property)
{
	Q_D(SlicerPropertySet);
	d->m_slicer->addProperty(key, property);
}

//END Pala::SlicerPropertySet

//BEGIN Pala::SimpleGridPropertySet

Pala::SimpleGridPropertySet::SimpleGridPropertySet(Pala::Slicer* slicer)
	: Pala::SlicerPropertySet(*new SimpleGridPropertySetPrivate(slicer))
{
	Pala::IntegerProperty* prop;
	prop = new Pala::IntegerProperty(i18n("Piece count"));
	prop->setRange(4, 10000);
	prop->setDefaultValue(100);
	addPropertyToSlicer("PieceCount", prop);
	prop = new Pala::IntegerProperty(i18n("Piece aspect ratio"));
	prop->setRange(0, 10);
	prop->setDefaultValue(5);
	prop->setRepresentation(Pala::IntegerProperty::Slider);
	addPropertyToSlicer("PieceAspect", prop);
}

Pala::SimpleGridPropertySet::~SimpleGridPropertySet() = default;

QSize Pala::SimpleGridPropertySet::pieceCount(Pala::SlicerJob* job) const
{
	const qreal imageAspect = qreal(job->image().width()) / qreal(job->image().height());
	const qreal pieceAspect = pow(2.0, qreal(job->argument("PieceAspect").toInt() - 5) * 0.2);
	const int count = job->argument("PieceCount").toInt();

	QSize bestCount(10, 10);
	qreal bestQ = 1.0e100;

	qreal aspect = imageAspect / pieceAspect;
	int maxX = ceil(sqrt(qreal(count) * aspect)) + 5;
	int maxY = ceil(sqrt(qreal(count) / aspect)) + 5;
	qCDebug(PALAPELI_LIBPALA_LOG) << "Determining counts for total count" << count;
	qCDebug(PALAPELI_LIBPALA_LOG) << "  Piece aspect ratio is" << pieceAspect;
	qCDebug(PALAPELI_LIBPALA_LOG) << "  Image aspect is" << imageAspect;
	qCDebug(PALAPELI_LIBPALA_LOG) << "  Target count aspect is" << aspect;
	qCDebug(PALAPELI_LIBPALA_LOG) << " Will try x <" << maxX << ", y <" << maxY;

	for (int x = 1; x < maxX; ++x)
	{
		for (int y = 1; y < maxY; ++y)
		{
			const int c = x * y;
			const qreal xa = x / aspect;
			const qreal a = pow((xa > y ? xa / qreal(y) : qreal(y) / xa) - 1.0, 2.0) * 20.0;
			const qreal dev = pow(6.0 * (count - c) / count, 2.0);
			const qreal q = (a + 1.0) * (dev + 1.0);

			if (q < bestQ)
			{
				bestQ = q;
				bestCount = QSize(x, y);
			}
		}
	}
	qCDebug(PALAPELI_LIBPALA_LOG) << "We liked " << bestCount << " ( at quality" << bestQ << ")";
	return bestCount;
}

//END Pala::SimpleGridPropertySet

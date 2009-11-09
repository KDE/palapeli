/***************************************************************************
 *   Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "slicerpropertyset.h"
#include "slicer.h"
#include "slicerjob.h"
#include "slicerproperty.h"

#include <cmath>
#include <QDebug>
#include <KLocalizedString>

//BEGIN Private classes

struct Pala::SlicerPropertySet::Private
{
	Pala::Slicer* m_slicer;
};

struct Pala::SimpleGridPropertySet::Private {};

//END Private classes

//BEGIN Pala::SlicerPropertySet

Pala::SlicerPropertySet::SlicerPropertySet(Pala::Slicer* slicer)
	: p(new Pala::SlicerPropertySet::Private)
{
	p->m_slicer = slicer;
}

Pala::SlicerPropertySet::~SlicerPropertySet()
{
	delete p;
}

Pala::Slicer* Pala::SlicerPropertySet::slicer() const
{
	return p->m_slicer;
}

void Pala::SlicerPropertySet::addPropertyToSlicer(const QByteArray& key, Pala::SlicerProperty* property)
{
	p->m_slicer->addProperty(key, property);
}

//END Pala::SlicerPropertySet

//BEGIN Pala::SimpleGridPropertySet

Pala::SimpleGridPropertySet::SimpleGridPropertySet(Pala::Slicer* slicer)
	: Pala::SlicerPropertySet(slicer)
	, p(0)
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

Pala::SimpleGridPropertySet::~SimpleGridPropertySet()
{
	delete p;
}

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
	qDebug() << "Determining counts for total count" << count;
	qDebug() << "  Piece aspect ratio is" << pieceAspect;
	qDebug() << "  Image aspect is" << imageAspect;
	qDebug() << "  Target count aspect is" << aspect;
	qDebug() << " Will try x <" << maxX << ", y <" << maxY;

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
	qDebug() << "We liked " << bestCount << " ( at quality" << bestQ << ")";
	return bestCount;
}

//END Pala::SimpleGridPropertySet

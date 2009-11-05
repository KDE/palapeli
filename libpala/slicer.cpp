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

#include "slicer.h"
#include "slicerjob.h"
#include "slicerproperty.h"

//BEGIN Pala::Slicer::Private

class Pala::Slicer::Private
{
	public:
		QMap<QByteArray, const Pala::SlicerProperty*> m_properties;
		Pala::Slicer::SlicerFlags m_flags;
};

//END Pala::Slicer::Private

Pala::Slicer::Slicer(QObject* parent, const QVariantList& /*args*/)
	: QObject(parent)
	, p(new Private)
{
	p->m_flags = NoFlags;
}

Pala::Slicer::~Slicer()
{
	qDeleteAll(p->m_properties);
	delete p;
}

QMap<QByteArray, const Pala::SlicerProperty*> Pala::Slicer::properties() const
{
	return p->m_properties;
}

Pala::Slicer::SlicerFlags Pala::Slicer::flags() const
{
	return p->m_flags;
}

void Pala::Slicer::addProperty(const QByteArray& key, Pala::SlicerProperty* property)
{
	delete p->m_properties[key]; //this is safe because m_properties[key] == 0 if key has not been used yet
	p->m_properties[key] = property;
}

void Pala::Slicer::setFlags(Pala::Slicer::SlicerFlags flags)
{
	p->m_flags = flags;
}

bool Pala::Slicer::process(Pala::SlicerJob* job)
{
	job->respectSlicerFlags(p->m_flags);
	return run(job);
}

#include "slicer.moc"

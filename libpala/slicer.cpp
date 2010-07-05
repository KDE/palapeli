/***************************************************************************
 *   Copyright 2009, 2010 Stefan Majewsky <majewsky@gmx.net>
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
#include "slicermode.h"
#include "slicerproperty.h"

//BEGIN Pala::Slicer::Private

class Pala::Slicer::Private
{
	public:
		QList<const Pala::SlicerMode*> m_modes;
		QList<const Pala::SlicerProperty*> m_properties;
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
	qDeleteAll(p->m_modes);
	qDeleteAll(p->m_properties);
	delete p;
}

QList<const Pala::SlicerMode*> Pala::Slicer::modes() const
{
	return p->m_modes;
}

QMap<QByteArray, const Pala::SlicerProperty*> Pala::Slicer::properties() const
{
	QMap<QByteArray, const Pala::SlicerProperty*> result;
	foreach (const Pala::SlicerProperty* property, p->m_properties)
		result.insert(property->key(), property);
	return result;
}

QList<const Pala::SlicerProperty*> Pala::Slicer::propertyList() const
{
	return p->m_properties;
}

Pala::Slicer::SlicerFlags Pala::Slicer::flags() const
{
	return p->m_flags;
}

void Pala::Slicer::addProperty(const QByteArray& key, Pala::SlicerProperty* property)
{
	//NOTE: This function is done such that it retains the insertion order in the list.
	for (int i = 0; i < p->m_properties.size(); ++i)
	{
		if (p->m_properties[i] == property)
			return;
		if (p->m_properties[i]->key() == key)
		{
			delete p->m_properties.takeAt(i);
			break;
		}
	}
	p->m_properties << property;
	property->setKey(key);
}

void Pala::Slicer::addMode(Pala::SlicerMode* mode)
{
	//NOTE: This one too. ;-)
	for (int i = 0; i < p->m_modes.size(); ++i)
	{
		if (p->m_modes[i] == mode)
			return;
		if (p->m_modes[i]->key() == mode->key())
		{
			delete p->m_modes.takeAt(i);
			break;
		}
	}
	p->m_modes << mode;
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

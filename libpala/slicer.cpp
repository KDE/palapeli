/*
    SPDX-FileCopyrightText: 2009, 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "slicer.h"
#include "slicerjob.h"
#include "slicermode.h"
#include "slicerproperty.h"

//BEGIN Pala::SlicerPrivate

class Pala::SlicerPrivate
{
	public:
		QList<const Pala::SlicerMode*> m_modes;
		QList<const Pala::SlicerProperty*> m_properties;
		Pala::Slicer::SlicerFlags m_flags;
};

//END Pala::SlicerPrivate

Pala::Slicer::Slicer(QObject* parent, const QVariantList& /*args*/)
	: QObject(parent)
	, d_ptr(new SlicerPrivate)
{
	Q_D(Slicer);
	d->m_flags = NoFlags;
}

Pala::Slicer::~Slicer()
{
	Q_D(Slicer);
	qDeleteAll(d->m_modes);
	qDeleteAll(d->m_properties);
}

QList<const Pala::SlicerMode*> Pala::Slicer::modes() const
{
	Q_D(const Slicer);
	return d->m_modes;
}

QMap<QByteArray, const Pala::SlicerProperty*> Pala::Slicer::properties() const
{
	Q_D(const Slicer);
	QMap<QByteArray, const Pala::SlicerProperty*> result;
	for (const Pala::SlicerProperty* property : d->m_properties)
		result.insert(property->key(), property);
	return result;
}

QList<const Pala::SlicerProperty*> Pala::Slicer::propertyList() const
{
	Q_D(const Slicer);
	return d->m_properties;
}

Pala::Slicer::SlicerFlags Pala::Slicer::flags() const
{
	Q_D(const Slicer);
	return d->m_flags;
}

void Pala::Slicer::addProperty(const QByteArray& key, Pala::SlicerProperty* property)
{
	Q_D(Slicer);
	//NOTE: This function is done such that it retains the insertion order in the list.
	for (int i = 0; i < d->m_properties.size(); ++i)
	{
		if (d->m_properties[i] == property)
			return;
		if (d->m_properties[i]->key() == key)
		{
			delete d->m_properties.takeAt(i);
			break;
		}
	}
	d->m_properties << property;
	property->setKey(key);
}

void Pala::Slicer::addMode(Pala::SlicerMode* mode)
{
	Q_D(Slicer);
	//NOTE: This one too. ;-)
	for (int i = 0; i < d->m_modes.size(); ++i)
	{
		if (d->m_modes[i] == mode)
			return;
		if (d->m_modes[i]->key() == mode->key())
		{
			delete d->m_modes.takeAt(i);
			break;
		}
	}
	d->m_modes << mode;
}

void Pala::Slicer::setFlags(Pala::Slicer::SlicerFlags flags)
{
	Q_D(Slicer);
	d->m_flags = flags;
}

bool Pala::Slicer::process(Pala::SlicerJob* job)
{
	Q_D(Slicer);
	job->respectSlicerFlags(d->m_flags);
	return run(job);
}



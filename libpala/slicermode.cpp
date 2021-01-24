/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "slicermode.h"
#include "slicerproperty.h"

class Pala::SlicerMode::Private
{
	public:
		QByteArray m_key;
		QString m_name;

		QHash<QByteArray, bool> m_propertyEnabledExceptions;
};

Pala::SlicerMode::SlicerMode(const QByteArray& key, const QString& name)
	: p(new Pala::SlicerMode::Private)
{
	p->m_key = key;
	p->m_name = name;
}

Pala::SlicerMode::~SlicerMode()
{
	delete p;
}

void Pala::SlicerMode::filterProperties(QList<const Pala::SlicerProperty*>& properties) const
{
	QMutableListIterator<const Pala::SlicerProperty*> iter(properties);
	while (iter.hasNext())
	{
		const Pala::SlicerProperty* property = iter.next();
		bool isEnabled = property->isEnabled();
		if (p->m_propertyEnabledExceptions.contains(property->key()))
			isEnabled = p->m_propertyEnabledExceptions.value(property->key());
		if (!isEnabled)
			iter.remove();
	}
}

QByteArray Pala::SlicerMode::key() const
{
	return p->m_key;
}

QString Pala::SlicerMode::name() const
{
	return p->m_name;
}

void Pala::SlicerMode::setPropertyEnabled(const QByteArray& property, bool enabled)
{
	p->m_propertyEnabledExceptions.insert(property, enabled);
}

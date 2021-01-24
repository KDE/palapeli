/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "slicermode.h"
#include "slicerproperty.h"

class Pala::SlicerModePrivate
{
	public:
		QByteArray m_key;
		QString m_name;

		QHash<QByteArray, bool> m_propertyEnabledExceptions;
};

Pala::SlicerMode::SlicerMode(const QByteArray& key, const QString& name)
	: d_ptr(new Pala::SlicerModePrivate)
{
	Q_D(SlicerMode);
	d->m_key = key;
	d->m_name = name;
}

Pala::SlicerMode::~SlicerMode() = default;

void Pala::SlicerMode::filterProperties(QList<const Pala::SlicerProperty*>& properties) const
{
	Q_D(const SlicerMode);
	QMutableListIterator<const Pala::SlicerProperty*> iter(properties);
	while (iter.hasNext())
	{
		const Pala::SlicerProperty* property = iter.next();
		bool isEnabled = property->isEnabled();
		if (d->m_propertyEnabledExceptions.contains(property->key()))
			isEnabled = d->m_propertyEnabledExceptions.value(property->key());
		if (!isEnabled)
			iter.remove();
	}
}

QByteArray Pala::SlicerMode::key() const
{
	Q_D(const SlicerMode);
	return d->m_key;
}

QString Pala::SlicerMode::name() const
{
	Q_D(const SlicerMode);
	return d->m_name;
}

void Pala::SlicerMode::setPropertyEnabled(const QByteArray& property, bool enabled)
{
	Q_D(SlicerMode);
	d->m_propertyEnabledExceptions.insert(property, enabled);
}

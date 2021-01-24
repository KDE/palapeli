/*
    SPDX-FileCopyrightText: 2009, 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "slicerproperty.h"

#include <QMutableListIterator>

//BEGIN private classes

class Pala::SlicerPropertyPrivate
{
public:
	SlicerPropertyPrivate(QVariant::Type type, const QString& caption)
	    : m_type(type)
	    , m_caption(caption)
	{
	}

	QVariant::Type const m_type;
	QString m_caption;
	QByteArray m_key;

	QVariantList m_choices;
	QVariant m_defaultValue;

	bool m_advanced = false;
	bool m_enabled = true;
};

class Pala::BooleanPropertyPrivate : public Pala::SlicerPropertyPrivate
{
public:
	BooleanPropertyPrivate(QVariant::Type type, const QString& caption)
	    : SlicerPropertyPrivate(type, caption)
	{
	}
};

class Pala::IntegerPropertyPrivate : public Pala::SlicerPropertyPrivate
{
public:
	IntegerPropertyPrivate(QVariant::Type type, const QString& caption)
	    : SlicerPropertyPrivate(type, caption)
	{
	}

	QPair<int, int> m_range;
	Pala::IntegerProperty::Representation m_representation;
};

class Pala::StringPropertyPrivate : public Pala::SlicerPropertyPrivate
{
public:
	StringPropertyPrivate(QVariant::Type type, const QString& caption)
	    : SlicerPropertyPrivate(type, caption)
	{
	}
};

//END private classes

//BEGIN Pala::SlicerProperty

Pala::SlicerProperty::SlicerProperty(Pala::SlicerPropertyPrivate& dd)
	: d_ptr(&dd)
{
}

Pala::SlicerProperty::~SlicerProperty() = default;

QString Pala::SlicerProperty::caption() const
{
	Q_D(const SlicerProperty);
	return d->m_caption;
}

QVariantList Pala::SlicerProperty::choices() const
{
	Q_D(const SlicerProperty);
	return d->m_choices;
}

QVariant Pala::SlicerProperty::defaultValue() const
{
	Q_D(const SlicerProperty);
	return d->m_defaultValue;
}

bool Pala::SlicerProperty::isAdvanced() const
{
	Q_D(const SlicerProperty);
	return d->m_advanced;
}

bool Pala::SlicerProperty::isEnabled() const
{
	Q_D(const SlicerProperty);
	return d->m_enabled;
}

QByteArray Pala::SlicerProperty::key() const
{
	Q_D(const SlicerProperty);
	return d->m_key;
}

QVariant::Type Pala::SlicerProperty::type() const
{
	Q_D(const SlicerProperty);
	return d->m_type;
}

void Pala::SlicerProperty::setAdvanced(bool advanced)
{
	Q_D(SlicerProperty);
	d->m_advanced = advanced;
}

void Pala::SlicerProperty::setChoices(const QVariantList& choices)
{
	Q_D(SlicerProperty);
	d->m_choices = choices;
	QMutableListIterator<QVariant> iter(d->m_choices);
	while (iter.hasNext())
		iter.next().convert(d->m_type);
}

void Pala::SlicerProperty::setDefaultValue(const QVariant& value)
{
	Q_D(SlicerProperty);
	d->m_defaultValue = value;
	d->m_defaultValue.convert(d->m_type);
}

void Pala::SlicerProperty::setEnabled(bool enabled)
{
	Q_D(SlicerProperty);
	d->m_enabled = enabled;
}

void Pala::SlicerProperty::setKey(const QByteArray& key)
{
	Q_D(SlicerProperty);
	d->m_key = key;
}

//END Pala::SlicerProperty

//BEGIN concrete implementations

Pala::BooleanProperty::BooleanProperty(const QString& caption)
	: Pala::SlicerProperty(*new Pala::BooleanPropertyPrivate(QVariant::Bool, caption))
{
}

Pala::BooleanProperty::~BooleanProperty() = default;

Pala::IntegerProperty::IntegerProperty(const QString& caption)
	: Pala::SlicerProperty(*new Pala::IntegerPropertyPrivate(QVariant::Int, caption))
{
	Q_D(IntegerProperty);
	d->m_range.first = d->m_range.second = 0;
	d->m_representation = Pala::IntegerProperty::DefaultRepresentation;
}

Pala::IntegerProperty::~IntegerProperty() = default;

QPair<int, int> Pala::IntegerProperty::range() const
{
	Q_D(const IntegerProperty);
	return d->m_range;
}

Pala::IntegerProperty::Representation Pala::IntegerProperty::representation() const
{
	Q_D(const IntegerProperty);
	return d->m_representation;
}

void Pala::IntegerProperty::setRange(int min, int max)
{
	Q_D(IntegerProperty);
	d->m_range.first = min;
	d->m_range.second = max;
}

void Pala::IntegerProperty::setRepresentation(Pala::IntegerProperty::Representation representation)
{
	Q_D(IntegerProperty);
	d->m_representation = representation;
}

Pala::StringProperty::StringProperty(const QString& caption)
	: Pala::SlicerProperty(*new Pala::StringPropertyPrivate(QVariant::String, caption))
{
}

Pala::StringProperty::~StringProperty() = default;

//END concrete implementations

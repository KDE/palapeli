/*
    SPDX-FileCopyrightText: 2009, 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "slicerproperty.h"

#include <QMutableListIterator>

//BEGIN private classes

struct Pala::SlicerProperty::Private
{
	Private() : m_advanced(false), m_enabled(true) {}

	QVariant::Type m_type;
	QString m_caption;
	QByteArray m_key;

	QVariantList m_choices;
	QVariant m_defaultValue;

	bool m_advanced, m_enabled;
};

struct Pala::BooleanProperty::Private {};

struct Pala::IntegerProperty::Private
{
	QPair<int, int> m_range;
	Pala::IntegerProperty::Representation m_representation;
};

struct Pala::StringProperty::Private {};

//END private classes

//BEGIN Pala::SlicerProperty

Pala::SlicerProperty::SlicerProperty(QVariant::Type type, const QString& caption)
	: p(new Pala::SlicerProperty::Private)
{
	p->m_type = type;
	p->m_caption = caption;
}

Pala::SlicerProperty::~SlicerProperty()
{
	delete p;
}

QString Pala::SlicerProperty::caption() const
{
	return p->m_caption;
}

QVariantList Pala::SlicerProperty::choices() const
{
	return p->m_choices;
}

QVariant Pala::SlicerProperty::defaultValue() const
{
	return p->m_defaultValue;
}

bool Pala::SlicerProperty::isAdvanced() const
{
	return p->m_advanced;
}

bool Pala::SlicerProperty::isEnabled() const
{
	return p->m_enabled;
}

QByteArray Pala::SlicerProperty::key() const
{
	return p->m_key;
}

QVariant::Type Pala::SlicerProperty::type() const
{
	return p->m_type;
}

void Pala::SlicerProperty::setAdvanced(bool advanced)
{
	p->m_advanced = advanced;
}

void Pala::SlicerProperty::setChoices(const QVariantList& choices)
{
	p->m_choices = choices;
	QMutableListIterator<QVariant> iter(p->m_choices);
	while (iter.hasNext())
		iter.next().convert(p->m_type);
}

void Pala::SlicerProperty::setDefaultValue(const QVariant& value)
{
	p->m_defaultValue = value;
	p->m_defaultValue.convert(p->m_type);
}

void Pala::SlicerProperty::setEnabled(bool enabled)
{
	p->m_enabled = enabled;
}

void Pala::SlicerProperty::setKey(const QByteArray& key)
{
	p->m_key = key;
}

//END Pala::SlicerProperty

//BEGIN concrete implementations

Pala::BooleanProperty::BooleanProperty(const QString& caption)
	: Pala::SlicerProperty(QVariant::Bool, caption)
	, p(nullptr)
{
}

Pala::BooleanProperty::~BooleanProperty()
{
	delete p;
}

Pala::IntegerProperty::IntegerProperty(const QString& caption)
	: Pala::SlicerProperty(QVariant::Int, caption)
	, p(new Pala::IntegerProperty::Private)
{
	p->m_range.first = p->m_range.second = 0;
	p->m_representation = Pala::IntegerProperty::DefaultRepresentation;
}

Pala::IntegerProperty::~IntegerProperty()
{
	delete p;
}

QPair<int, int> Pala::IntegerProperty::range() const
{
	return p->m_range;
}

Pala::IntegerProperty::Representation Pala::IntegerProperty::representation() const
{
	return p->m_representation;
}

void Pala::IntegerProperty::setRange(int min, int max)
{
	p->m_range.first = min;
	p->m_range.second = max;
}

void Pala::IntegerProperty::setRepresentation(Pala::IntegerProperty::Representation representation)
{
	p->m_representation = representation;
}

Pala::StringProperty::StringProperty(const QString& caption)
	: Pala::SlicerProperty(QVariant::String, caption)
	, p(nullptr)
{
}

Pala::StringProperty::~StringProperty()
{
	delete p;
}

//END concrete implementations

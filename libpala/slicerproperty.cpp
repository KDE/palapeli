/***************************************************************************
 *   Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "slicerproperty.h"

#include <QStringList>

//BEGIN Pala::SlicerProperty::Private

class Pala::SlicerProperty::Private
{
	public:
		Pala::SlicerProperty::Type m_type;
		QString m_caption;

		QStringList m_choices;
		QPair<int, int> m_range;
		QVariant m_defaultValue;
};

//END Pala::SlicerProperty::Private

Pala::SlicerProperty::SlicerProperty(Pala::SlicerProperty::Type type, const QString& caption)
	: p(new Private)
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

QStringList Pala::SlicerProperty::choices() const
{
	return p->m_choices;
}

QVariant Pala::SlicerProperty::defaultValue() const
{
	return p->m_defaultValue;
}

int Pala::SlicerProperty::rangeMinimum() const
{
	return p->m_range.first;
}

int Pala::SlicerProperty::rangeMaximum() const
{
	return p->m_range.second;
}

Pala::SlicerProperty::Type Pala::SlicerProperty::type() const
{
	return p->m_type;
}

void Pala::SlicerProperty::setChoices(const QStringList& strings)
{
	Q_ASSERT_X(p->m_type == String, "Pala::SlicerProperty::setChoices", "wrong property type");
	p->m_choices = strings;
}

void Pala::SlicerProperty::setChoices(const QList<int>& numbers)
{
	Q_ASSERT_X(p->m_type == Integer, "Pala::SlicerProperty::setChoices", "wrong property type");
	p->m_choices.clear();
	foreach (int number, numbers)
		p->m_choices << QString::number(number);
}

void Pala::SlicerProperty::setDefaultValue(const QVariant& value)
{
	p->m_defaultValue = value;
}

void Pala::SlicerProperty::setRange(int min, int max)
{
	Q_ASSERT_X(p->m_type == Integer, "Pala::SlicerProperty::setRange", "wrong property type");
	p->m_range.first = min;
	p->m_range.second = max;
}

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

//BEGIN Palapeli::SlicerProperty::Private

class Palapeli::SlicerProperty::Private
{
	public:
		Palapeli::SlicerProperty::Type m_type;
		QString m_caption;

		QStringList m_choices;
		QPair<int, int> m_range;
		QVariant m_defaultValue;
};

//END Palapeli::SlicerProperty::Private

Palapeli::SlicerProperty::SlicerProperty(Palapeli::SlicerProperty::Type type, const QString& caption)
	: p(new Private)
{
	p->m_type = type;
	p->m_caption = caption;
}

Palapeli::SlicerProperty::~SlicerProperty()
{
	delete p;
}

QString Palapeli::SlicerProperty::caption() const
{
	return p->m_caption;
}

QStringList Palapeli::SlicerProperty::choices() const
{
	return p->m_choices;
}

QVariant Palapeli::SlicerProperty::defaultValue() const
{
	return p->m_defaultValue;
}

int Palapeli::SlicerProperty::rangeMinimum() const
{
	return p->m_range.first;
}

int Palapeli::SlicerProperty::rangeMaximum() const
{
	return p->m_range.second;
}

Palapeli::SlicerProperty::Type Palapeli::SlicerProperty::type() const
{
	return p->m_type;
}

void Palapeli::SlicerProperty::setChoices(const QStringList& strings)
{
	Q_ASSERT_X(p->m_type == String, "Palapeli::SlicerProperty::setChoices", "wrong property type");
	p->m_choices = strings;
}

void Palapeli::SlicerProperty::setChoices(const QList<int>& numbers)
{
	Q_ASSERT_X(p->m_type == Integer, "Palapeli::SlicerProperty::setChoices", "wrong property type");
	p->m_choices.clear();
	foreach (int number, numbers)
		p->m_choices << QString::number(number);
}

void Palapeli::SlicerProperty::setDefaultValue(const QVariant& value)
{
	p->m_defaultValue = value;
}

void Palapeli::SlicerProperty::setRange(int min, int max)
{
	Q_ASSERT_X(p->m_type == Integer, "Palapeli::SlicerProperty::setRange", "wrong property type");
	p->m_range.first = min;
	p->m_range.first = max;
}

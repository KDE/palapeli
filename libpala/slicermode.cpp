/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>
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

#include "slicermode.h"
#include "slicerproperty.h"

class Pala::SlicerMode::Private
{
	public:
		QHash<QByteArray, bool> m_propertyEnabledExceptions;
};

Pala::SlicerMode::SlicerMode()
	: p(new Pala::SlicerMode::Private)
{
}

Pala::SlicerMode::~SlicerMode()
{
	delete p;
}

void Pala::SlicerMode::filterProperties(QMap<QByteArray, const Pala::SlicerProperty*>& properties) const
{
	QMutableMapIterator<QByteArray, const Pala::SlicerProperty*> iter(properties);
	while (iter.hasNext())
	{
		const QByteArray key = iter.next().key();
		bool isEnabled = iter.value()->isEnabled();
		if (p->m_propertyEnabledExceptions.contains(key))
			isEnabled = p->m_propertyEnabledExceptions.value(key);
		if (!isEnabled)
			iter.remove();
	}
}

void Pala::SlicerMode::setPropertyEnabled(const QByteArray& property, bool enabled)
{
	p->m_propertyEnabledExceptions.insert(property, enabled);
}

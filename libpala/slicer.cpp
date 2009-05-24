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

#include "slicer.h"
#include "slicerproperty.h"

//BEGIN Palapeli::Slicer::Private

class Palapeli::Slicer::Private
{
	public:
		QMap<QByteArray, const Palapeli::SlicerProperty*> m_properties;
};

//END Palapeli::Slicer::Private

Palapeli::Slicer::Slicer(QObject* parent, const QVariantList& /*args*/)
	: QObject(parent)
	, p(new Private)
{
}

Palapeli::Slicer::~Slicer()
{
	delete p;
}

QMap<QByteArray, const Palapeli::SlicerProperty*> Palapeli::Slicer::properties() const
{
	return p->m_properties;
}

void Palapeli::Slicer::addProperty(const QByteArray& key, Palapeli::SlicerProperty* property)
{
	delete p->m_properties[key]; //this is safe because m_properties[key] == 0 if key has not been used yet
	p->m_properties[key] = property;
}

#include "slicer.moc"

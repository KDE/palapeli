/***************************************************************************
 *   Copyright 2009, 2010 Stefan Majewsky <majewsky@gmx.net>
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

#include "slicerconfwidget.h"
#include "propertywidget.h"
#include "../libpala/slicer.h"
#include "../libpala/slicerproperty.h"

#include <QFormLayout>

Palapeli::SlicerConfigWidget::SlicerConfigWidget(Pala::Slicer* slicer)
{
	QFormLayout* layout = new QFormLayout;
	setLayout(layout);
	//create property widgets
	typedef QPair<QByteArray, const Pala::SlicerProperty*> PropertyPair;
	QList<PropertyPair> properties = slicer->propertyList();
	foreach (const PropertyPair& propPair, properties)
	{
		const Pala::SlicerProperty* property = propPair.second;
		Palapeli::PropertyWidget* propWidget = Palapeli::createPropertyWidget(property);
		m_propertyWidgets[propPair.first] = propWidget;
		layout->addRow(property->caption() + QChar(':'), propWidget);
	}
}

QMap<QByteArray, QVariant> Palapeli::SlicerConfigWidget::arguments() const
{
	QMap<QByteArray, QVariant> result;
	QMapIterator<QByteArray, Palapeli::PropertyWidget*> i(m_propertyWidgets);
	while (i.hasNext())
	{
		const Palapeli::PropertyWidget* propWidget = i.next().value();
		result[i.key()] = propWidget->propertyValue();
	}
	return result;
}

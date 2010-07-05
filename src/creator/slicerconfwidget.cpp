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
#include "../libpala/slicermode.h"
#include "../libpala/slicerproperty.h"

#include <QFormLayout>

Palapeli::SlicerConfigWidget::SlicerConfigWidget(const Pala::Slicer* slicer)
	: m_layout(new QFormLayout)
{
	setLayout(m_layout);
	//create property widgets
	QList<const Pala::SlicerProperty*> properties = slicer->propertyList();
	foreach (const Pala::SlicerProperty* property, properties)
	{
		Palapeli::PropertyWidget* propWidget = Palapeli::createPropertyWidget(property);
		Entry entry = { property, propWidget };
		m_entries << entry;
		m_layout->addRow(property->caption() + QChar(':'), propWidget);
	}
}

QMap<QByteArray, QVariant> Palapeli::SlicerConfigWidget::arguments() const
{
	QMap<QByteArray, QVariant> result;
	foreach (const Entry& entry, m_entries)
		result.insert(entry.property->key(), entry.widget->propertyValue());
	return result;
}

void Palapeli::SlicerConfigWidget::setMode(const Pala::SlicerMode* mode)
{
	//determine enabled properties
	QList<const Pala::SlicerProperty*> enabledProps;
	foreach (const Entry& entry, m_entries)
		enabledProps << entry.property;
	if (mode)
		mode->filterProperties(enabledProps);
	//update widget visibility according to enabled/disabled state
	foreach (const Entry& entry, m_entries)
	{
		const bool isVisible = enabledProps.contains(entry.property);
		entry.widget->setVisible(isVisible);
		m_layout->labelForField(entry.widget)->setVisible(isVisible);
	}
}

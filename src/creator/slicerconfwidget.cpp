/*
    SPDX-FileCopyrightText: 2009, 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "slicerconfwidget.h"
#include "propertywidget.h"

#include <Pala/Slicer>
#include <Pala/SlicerMode>
#include <Pala/SlicerProperty>

#include <QFormLayout>

Palapeli::SlicerConfigWidget::SlicerConfigWidget(const Pala::Slicer* slicer)
	: m_layout(new QFormLayout)
{
	setLayout(m_layout);
	//create property widgets
	const QList<const Pala::SlicerProperty*> properties = slicer->propertyList();
	for (const Pala::SlicerProperty* property : properties)
	{
		Palapeli::PropertyWidget* propWidget = Palapeli::createPropertyWidget(property);
		Entry entry = { property, propWidget };
		m_entries << entry;
		m_layout->addRow(property->caption() + QLatin1Char(':'), propWidget);
	}
}

QMap<QByteArray, QVariant> Palapeli::SlicerConfigWidget::arguments() const
{
	QMap<QByteArray, QVariant> result;
	for (const Entry& entry : m_entries)
		result.insert(entry.property->key(), entry.widget->propertyValue());
	return result;
}

void Palapeli::SlicerConfigWidget::setMode(const Pala::SlicerMode* mode)
{
	//determine enabled properties
	QList<const Pala::SlicerProperty*> enabledProps;
	for (const Entry& entry : qAsConst(m_entries))
		enabledProps << entry.property;
	if (mode)
		mode->filterProperties(enabledProps);
	//update widget visibility according to enabled/disabled state
	for (const Entry& entry : qAsConst(m_entries))
	{
		const bool isVisible = enabledProps.contains(entry.property);
		entry.widget->setVisible(isVisible);
		m_layout->labelForField(entry.widget)->setVisible(isVisible);
	}
}

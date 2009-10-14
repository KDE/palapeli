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

#include "propertywidget.h"
#include "propertywidget_p.h"
#include "../libpala/slicerproperty.h"

#include <QHBoxLayout>

Palapeli::PropertyWidget* Palapeli::createPropertyWidget(const Pala::SlicerProperty* property)
{
	Palapeli::PropertyWidget* pw;
	switch (property->type())
	{
		case Pala::SlicerProperty::Boolean:
			pw = new Palapeli::BooleanPropertyWidget;
			break;
		case Pala::SlicerProperty::Integer:
			pw = new Palapeli::IntegerPropertyWidget;
			break;
		case Pala::SlicerProperty::String:
			pw = new Palapeli::StringPropertyWidget;
			break;
		default:
			return 0;
	}
	pw->initialize(property);
	return pw;
}

//BEGIN Palapeli::BooleanPropertyWidget

Palapeli::BooleanPropertyWidget::BooleanPropertyWidget()
	: m_checkBox(0)
{
}

void Palapeli::BooleanPropertyWidget::initialize(const Pala::SlicerProperty* property)
{
	m_checkBox = new QCheckBox(this);
	m_checkBox->setChecked(property->defaultValue().toBool());
	QHBoxLayout* layout = new QHBoxLayout;
	layout->addWidget(m_checkBox);
	layout->setMargin(0);
	setLayout(layout);
}

QVariant Palapeli::BooleanPropertyWidget::propertyValue() const
{
	return m_checkBox ? QVariant(m_checkBox->isChecked()) : QVariant();
}

//END Palapeli::BooleanPropertyWidget

//BEGIN Palapeli::IntegerPropertyWidget

Palapeli::IntegerPropertyWidget::IntegerPropertyWidget()
	: m_comboBox(0)
	, m_spinBox(0)
{
}

void Palapeli::IntegerPropertyWidget::initialize(const Pala::SlicerProperty* property)
{
	const QPair<int,int> range = qMakePair(property->rangeMinimum(), property->rangeMaximum());
	const QStringList choices = property->choices();
	QWidget* usedWidget;
	if (choices.isEmpty())
	{
		usedWidget = m_spinBox = new KIntSpinBox(this);
		if (range.first != range.second) //only set range if it is not empty
			m_spinBox->setRange(range.first, range.second);
		m_spinBox->setValue(property->defaultValue().toInt());
	}
	else
	{
		usedWidget = m_comboBox = new KComboBox(false, this); //false = not editable (only given choices can be used)
		m_comboBox->addItems(choices);
	}
	QHBoxLayout* layout = new QHBoxLayout;
	layout->addWidget(usedWidget);
	layout->setMargin(0);
	setLayout(layout);
}

QVariant Palapeli::IntegerPropertyWidget::propertyValue() const
{
	return m_spinBox ? QVariant(m_spinBox->value()) : (m_comboBox ? QVariant(m_comboBox->currentText()) : QVariant());
}

//END Palapeli::IntegerPropertyWidget

//BEGIN Palapeli::StringPropertyWidget

Palapeli::StringPropertyWidget::StringPropertyWidget()
	: m_comboBox(0)
	, m_lineEdit(0)
{
}

void Palapeli::StringPropertyWidget::initialize(const Pala::SlicerProperty* property)
{
	const QStringList choices = property->choices();
	QWidget* usedWidget;
	if (choices.isEmpty())
		usedWidget = m_lineEdit = new KLineEdit(property->defaultValue().toString(), this);
	else
	{
		usedWidget = m_comboBox = new KComboBox(false, this); //false = not editable (only given choices can be used)
		m_comboBox->addItems(choices);
	}
	QHBoxLayout* layout = new QHBoxLayout;
	layout->addWidget(usedWidget);
	layout->setMargin(0);
	setLayout(layout);
}

QVariant Palapeli::StringPropertyWidget::propertyValue() const
{
	return m_lineEdit ? QVariant(m_lineEdit->text()) : (m_comboBox ? QVariant(m_comboBox->currentText()) : QVariant());
}

//END Palapeli::StringPropertyWidget

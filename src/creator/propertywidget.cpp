/*
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "propertywidget.h"
#include "propertywidget_p.h"

#include <Pala/SlicerProperty>

#include <QHBoxLayout>

Palapeli::PropertyWidget* Palapeli::createPropertyWidget(const Pala::SlicerProperty* property)
{
	Palapeli::PropertyWidget* pw;
	switch (property->type())
	{
		case QVariant::Bool:
			pw = new Palapeli::BooleanPropertyWidget;
			break;
		case QVariant::Int:
			pw = new Palapeli::IntegerPropertyWidget;
			break;
		case QVariant::String:
			pw = new Palapeli::StringPropertyWidget;
			break;
		default:
			return nullptr;
	}
	pw->initialize(property);
	return pw;
}

//BEGIN Palapeli::BooleanPropertyWidget

Palapeli::BooleanPropertyWidget::BooleanPropertyWidget()
	: m_checkBox(nullptr)
{
}

void Palapeli::BooleanPropertyWidget::initialize(const Pala::SlicerProperty* property)
{
	m_checkBox = new QCheckBox(this);
	m_checkBox->setChecked(property->defaultValue().toBool());
	QHBoxLayout* layout = new QHBoxLayout;
	layout->addWidget(m_checkBox);
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);
}

QVariant Palapeli::BooleanPropertyWidget::propertyValue() const
{
	return m_checkBox ? QVariant(m_checkBox->isChecked()) : QVariant();
}

//END Palapeli::BooleanPropertyWidget

//BEGIN Palapeli::IntegerPropertyWidget

Palapeli::IntegerPropertyWidget::IntegerPropertyWidget()
	: m_comboBox(nullptr)
	, m_spinBox(nullptr)
	, m_slider(nullptr)
{
}

void Palapeli::IntegerPropertyWidget::initialize(const Pala::SlicerProperty* property)
{
	const Pala::IntegerProperty* intProperty = static_cast<const Pala::IntegerProperty*>(property);
	const QPair<int,int> range = intProperty->range();
	const QVariantList choices = property->choices();
	QWidget* usedWidget;
	if (choices.isEmpty())
	{
		switch (intProperty->representation())
		{
			case Pala::IntegerProperty::SpinBox:
				usedWidget = m_spinBox = new QSpinBox(this);
				if (range.first != range.second) //only set range if it is not empty
					m_spinBox->setRange(range.first, range.second);
				m_spinBox->setValue(property->defaultValue().toInt());
				break;
			case Pala::IntegerProperty::Slider:
				usedWidget = m_slider = new QSlider(Qt::Horizontal, this);
				if (range.first != range.second) //only set range if it is not empty
					m_slider->setRange(range.first, range.second);
				m_slider->setValue(property->defaultValue().toInt());
				break;
		}
	}
	else
	{
		usedWidget = m_comboBox = new KComboBox(false, this); //false = not editable (only given choices can be used)
		for (const QVariant& choice : choices)
			m_comboBox->addItem(choice.toString());
	}
	QHBoxLayout* layout = new QHBoxLayout;
	layout->addWidget(usedWidget);
	usedWidget->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);
}

QVariant Palapeli::IntegerPropertyWidget::propertyValue() const
{
	if (m_spinBox)
		return m_spinBox->value();
	else if (m_slider)
		return m_slider->value();
	else if (m_comboBox)
		return m_comboBox->currentText();
	else //This may never happen.
		return QVariant();
}

//END Palapeli::IntegerPropertyWidget

//BEGIN Palapeli::StringPropertyWidget

Palapeli::StringPropertyWidget::StringPropertyWidget()
	: m_comboBox(nullptr)
	, m_lineEdit(nullptr)
{
}

void Palapeli::StringPropertyWidget::initialize(const Pala::SlicerProperty* property)
{
	const QVariantList choices = property->choices();
	QWidget* usedWidget;
	if (choices.isEmpty())
		usedWidget = m_lineEdit = new KLineEdit(property->defaultValue().toString(), this);
	else
	{
		usedWidget = m_comboBox = new KComboBox(false, this); //false = not editable (only given choices can be used)
		for (const QVariant& choice : choices)
			m_comboBox->addItem(choice.toString());
	}
	QHBoxLayout* layout = new QHBoxLayout;
	layout->addWidget(usedWidget);
	usedWidget->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);
}

QVariant Palapeli::StringPropertyWidget::propertyValue() const
{
	return m_lineEdit ? QVariant(m_lineEdit->text()) : (m_comboBox ? QVariant(m_comboBox->currentText()) : QVariant());
}

//END Palapeli::StringPropertyWidget

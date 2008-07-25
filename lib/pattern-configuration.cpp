/***************************************************************************
 *   Copyright (C) 2008 Stefan Majewsky <majewsky@gmx.net>
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

#include "pattern-configuration.h"
#include "variantmapper.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
#include <QMap>
#include <QWidget>
#include <KComboBox>
#include <KConfigGroup>
#include <KIntSpinBox>
#include <KLineEdit>
#include <KLocalizedString>

namespace Palapeli
{

	class PatternConfigurationPrivate
	{
		public:
			PatternConfigurationPrivate();
			~PatternConfigurationPrivate();

			QMap<QByteArray, QVariant> m_configurationValues;
			QMap<QByteArray, PatternConfiguration::DataType> m_configurationDataTypes;
			QMap<QByteArray, QString> m_configurationCaptions;
			QMap<QByteArray, QVariantList> m_configurationParameters;
			Palapeli::VariantMapper m_mapper;
	};

}

//BEGIN Palapeli::PatternConfigurationPrivate

Palapeli::PatternConfigurationPrivate::PatternConfigurationPrivate()
{
	m_configurationValues["patternName"] = QString();
	m_configurationDataTypes["patternName"] = Palapeli::PatternConfiguration::String;
	m_configurationValues["displayName"] = QString();
	m_configurationDataTypes["displayName"] = Palapeli::PatternConfiguration::String;
}

Palapeli::PatternConfigurationPrivate::~PatternConfigurationPrivate()
{
	//the QMaps are not flushed
}

//END Palapeli::PatternConfigurationPrivate

//BEGIN Palapeli::PatternConfiguration

Palapeli::PatternConfiguration::PatternConfiguration(QObject* parent, const QVariantList& args)
	: p(new Palapeli::PatternConfigurationPrivate)
{
	Q_UNUSED(parent)
	Q_UNUSED(args)
	connect(&p->m_mapper, SIGNAL(mapped(const QByteArray&, const QVariant&)), this, SLOT(setProperty(const QByteArray&, const QVariant&)));
}

Palapeli::PatternConfiguration::~PatternConfiguration()
{
	delete p;
}

QVariant Palapeli::PatternConfiguration::property(const QByteArray& key) const
{
	QVariant value = p->m_configurationValues.value(key, QVariant());
	switch (p->m_configurationDataTypes.value(key, Variant))
	{
		case String: value.convert(QVariant::String); return value;
		case Integer: value.convert(QVariant::Int); return value;
		case Boolean: value.convert(QVariant::Bool); return value;
		case Variant: default: return value;
	}
}

void Palapeli::PatternConfiguration::setProperty(const QByteArray& key, const QVariant& value)
{
	if (!p->m_configurationValues.contains(key))
		return;
	p->m_configurationValues[key] = value;
}

void Palapeli::PatternConfiguration::addProperty(const QByteArray& key, Palapeli::PatternConfiguration::DataType type, const QString& caption)
{
	if (p->m_configurationValues.contains(key))
		return;
	p->m_configurationValues[key] = QVariant();
	p->m_configurationDataTypes[key] = type;
	p->m_configurationCaptions[key] = caption;
	p->m_configurationParameters[key] = QVariantList();
}

void Palapeli::PatternConfiguration::addPropertyParameters(const QByteArray& key, const QVariantList& parameters)
{
	p->m_configurationParameters[key] = parameters;
}

void Palapeli::PatternConfiguration::readArguments(KConfigGroup* config)
{
	QMutableMapIterator<QByteArray, QVariant> iterConfigValues(p->m_configurationValues);
	while (iterConfigValues.hasNext())
	{
		QByteArray key = iterConfigValues.next().key();
		if (p->m_configurationCaptions.contains(key)) //do not read internal values (patternName and displayName)
			iterConfigValues.value() = config->readEntry(key.data(), QString());
	}
}

void Palapeli::PatternConfiguration::writeArguments(KConfigGroup* config) const
{
	QMapIterator<QByteArray, QVariant> iterConfigValues(p->m_configurationValues);
	while (iterConfigValues.hasNext())
	{
		QByteArray key = iterConfigValues.next().key();
		if (p->m_configurationCaptions.contains(key)) //do not write internal values (patternName and displayName)
			config->writeEntry(key.data(), property(key));
	}
}

void Palapeli::PatternConfiguration::readCustomArguments(KConfigGroup* config)
{
	Q_UNUSED(config)
}

void Palapeli::PatternConfiguration::writeCustomArguments(KConfigGroup* config) const
{
	Q_UNUSED(config)
}

void Palapeli::PatternConfiguration::populateWidget(QWidget* parentWidget)
{
	//temporary variables
	QWidget* widget = 0;
	KIntSpinBox* spinner = 0;
	QCheckBox* checker = 0;
	KLineEdit* lineEditor = 0;
	KComboBox* combo = 0;
	int currentIndex = -1;
	//create layout with configuration widgets in it
	QFormLayout* layout = new QFormLayout;
	QMapIterator<QByteArray, QString> iterConfigCaptions(p->m_configurationCaptions); //iterate over this to exclude everything which should not be visible to the user
	while (iterConfigCaptions.hasNext())
	{
		QByteArray key = iterConfigCaptions.next().key();
		QVariant value = p->m_configurationValues[key];
		QVariantList params = p->m_configurationParameters.value(key);
		switch (p->m_configurationDataTypes[key])
		{
			case Integer:
				widget = spinner = new KIntSpinBox(parentWidget);
				spinner->setMinimum(params.value(0, 0).toInt());
				spinner->setMaximum(params.value(1, 100).toInt());
				spinner->setValue(value.toInt());
				connect(spinner, SIGNAL(valueChanged(int)), &p->m_mapper, SLOT(map(int)));
				break;
			case Boolean:
				widget = checker = new QCheckBox(parentWidget);
				checker->setChecked(value.toBool());
				connect(checker, SIGNAL(toggled(bool)), &p->m_mapper, SLOT(map(bool)));
				break;
			case String: case Variant:
				if (params.isEmpty())
				{
					widget = lineEditor = new KLineEdit(parentWidget);
					lineEditor->setText(value.toString());
					connect(lineEditor, SIGNAL(textChanged(const QString&)), &p->m_mapper, SLOT(map(const QString&)));
				}
				else //use parameters as possible values
				{
					widget = combo = new KComboBox(parentWidget);
					//while adding the options, try to locate the currently selected one
					currentIndex = -1;
					foreach (QVariant option, params)
					{
						combo->addItem(option.toString());
						if (option.toString() == value.toString())
							currentIndex = combo->count() - 1; //because the item was inserted at the end of the list
					}
					if (currentIndex != -1)
						combo->setCurrentIndex(currentIndex);
					connect(combo, SIGNAL(currentIndexChanged(const QString&)), &p->m_mapper, SLOT(map(const QString&)));
				}
				break;
		}
		p->m_mapper.addMapping(widget, key);
		layout->addRow(iterConfigCaptions.value(), widget);
	}
	parentWidget->setLayout(layout);
}

//END Palapeli::PatternConfiguration

#include "pattern-configuration.moc"

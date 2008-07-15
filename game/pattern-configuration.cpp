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

#include <QFormLayout>
#include <QLabel>
#include <QMap>
#include <QPointer>
#include <QWidget>
#include <KConfigGroup>
#include <KIntSpinBox>
#include <KLocalizedString>

namespace Palapeli
{

	class PatternConfigurationPrivate
	{
		public:
			PatternConfigurationPrivate(const QByteArray& patternName, const QString& displayName);
			~PatternConfigurationPrivate();

			QByteArray m_patternName;
			QString m_displayName;

			KIntSpinBox* m_xCountSpinner;
			KIntSpinBox* m_yCountSpinner;
			QMap<QWidget*, QString> m_configurationWidgets;
	};

}

//BEGIN Palapeli::PatternConfigurationPrivate

Palapeli::PatternConfigurationPrivate::PatternConfigurationPrivate(const QByteArray& patternName, const QString& displayName)
	: m_patternName(patternName)
	, m_displayName(displayName)
	, m_xCountSpinner(0) //this makes CustomSizeDefinition the implicit default for SizeDefinitionMode of PatternConfiguration
	, m_yCountSpinner(0)
{
}

Palapeli::PatternConfigurationPrivate::~PatternConfigurationPrivate()
{
	delete m_xCountSpinner;
	delete m_yCountSpinner;
	//the configuration map is not flushed
}

//END Palapeli::PatternConfigurationPrivate

//BEGIN Palapeli::PatternConfiguration

Palapeli::PatternConfiguration::PatternConfiguration(const QByteArray& patternName, const QString& displayName)
	: p(new Palapeli::PatternConfigurationPrivate(patternName, displayName))
{
}

Palapeli::PatternConfiguration::~PatternConfiguration()
{
	delete p;
}

void Palapeli::PatternConfiguration::addWidget(QWidget* widget, const QString& caption)
{
	//do not add a widget twice
	if (p->m_configurationWidgets.contains(widget))
		return;
	p->m_configurationWidgets[widget] = caption;
}

void Palapeli::PatternConfiguration::removeWidget(QWidget* widget)
{
	p->m_configurationWidgets.take(widget);
}

void Palapeli::PatternConfiguration::setSizeDefinitionMode(SizeDefinitionMode mode)
{
	//find out old mode
	SizeDefinitionMode oldMode = (p->m_xCountSpinner == 0) ? CustomSizeDefinition : CountSizeDefinition;
	if (oldMode == mode)
		return; //nothing to do
	//set new mode
	switch (mode)
	{
		case CustomSizeDefinition:
			//coming from CountSizeDefinition - remove spin boxes (it is important that the pointer is set to 0 because this is used as condition for the CustomSizeDefinition in this code file)
			removeWidget(p->m_xCountSpinner);
			delete p->m_xCountSpinner;
			p->m_xCountSpinner = 0;
			removeWidget(p->m_yCountSpinner);
			delete p->m_yCountSpinner;
			p->m_yCountSpinner = 0;
			break;
		case CountSizeDefinition:
			//coming from CustomSizeDefinition - create and add spin boxes
			p->m_xCountSpinner = new KIntSpinBox(0, 100, 1, 10, 0); //parameters: min, max, step, value, parent
			addWidget(p->m_xCountSpinner, i18n("Piece count in horizontal direction:"));
			p->m_yCountSpinner = new KIntSpinBox(0, 100, 1, 10, 0);
			addWidget(p->m_yCountSpinner, i18n("Piece count in vertical direction:"));
			break;
	}
}

int Palapeli::PatternConfiguration::xCount() const
{
	if (p->m_xCountSpinner == 0)
		return -1; //logically invalid call (requesting CountSize but size mode not set to CountSizeDefinition)
	else
		return p->m_xCountSpinner->value();
}

int Palapeli::PatternConfiguration::yCount() const
{
	if (p->m_yCountSpinner == 0)
		return -1;
	else
		return p->m_yCountSpinner->value();
}

QByteArray Palapeli::PatternConfiguration::patternName() const
{
	return p->m_patternName;
}

QString Palapeli::PatternConfiguration::displayName() const
{
	return p->m_displayName;
}

QWidget* Palapeli::PatternConfiguration::createConfigurationWidget() const
{
	//build layout
	QFormLayout* layout = new QFormLayout;
	QMapIterator<QWidget*, QString> iterConfigWidgets(p->m_configurationWidgets);
	while (iterConfigWidgets.hasNext())
	{
		iterConfigWidgets.next();
		layout->addRow(iterConfigWidgets.value(), iterConfigWidgets.key());
	}
	//build widget
	QWidget* parentWidget = new QWidget;
	parentWidget->setLayout(layout);
	return parentWidget;
}

void Palapeli::PatternConfiguration::readArgumentsBase(KConfigGroup* config)
{
	if (p->m_xCountSpinner != 0)
		p->m_xCountSpinner->setValue(config->readEntry("XCount", 10));
	if (p->m_yCountSpinner != 0)
		p->m_yCountSpinner->setValue(config->readEntry("YCount", 10));
}

void Palapeli::PatternConfiguration::writeArgumentsBase(KConfigGroup* config) const
{
	if (p->m_xCountSpinner != 0)
		config->writeEntry("XCount", p->m_xCountSpinner->value());
	if (p->m_yCountSpinner != 0)
		config->writeEntry("YCount", p->m_yCountSpinner->value());
}

//END Palapeli::PatternConfiguration

#include "pattern-configuration.moc"

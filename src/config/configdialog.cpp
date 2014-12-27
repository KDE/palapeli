/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>
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

#include "configdialog.h"
#include "configdialog_p.h"
#include "triggerconfigwidget.h"
#include "../engine/texturehelper.h"
#include "settings.h"
#include <QIcon>
#include <QTimer>

//BEGIN Palapeli::TriggerComboBox

Palapeli::TriggerComboBox::TriggerComboBox(QWidget* parent)
	: KComboBox(parent)
{
	connect(this, SIGNAL(currentIndexChanged(int)), SLOT(handleCurrentIndexChanged(int)));
}

QString Palapeli::TriggerComboBox::backgroundKey() const
{
	return itemData(currentIndex(), Palapeli::TextureHelper::IdentifierRole).toString();
}

void Palapeli::TriggerComboBox::setBackgroundKey(const QString& backgroundKey)
{
	int index = findData(backgroundKey, Palapeli::TextureHelper::IdentifierRole);
	if (index > -1)
		setCurrentIndex(index);
}

void Palapeli::TriggerComboBox::handleCurrentIndexChanged(int index)
{
	Q_UNUSED(index)
	const QString key = backgroundKey();
	emit backgroundKeyChanged(key);
	emit itemTypeChanged(key == QLatin1String("__color__"));
}

//END Palapeli::TriggerComboBox
//BEGIN Palapeli::ConfigDialog

Palapeli::ConfigDialog::ConfigDialog(QWidget* parent)
	: KConfigDialog(parent, QString(), Settings::self())
	, m_triggerPage(new Palapeli::TriggerConfigWidget)
	, m_shownForFirstTime(false)
{
	//setup page "General settings"
	QWidget* generalPage = new QWidget;
	m_generalUi.setupUi(generalPage);

	// Construction of the TextureHelper singleton also loads all Palapeli
	// settings or defaults and the combobox contents for ViewBackground.
	m_generalUi.kcfg_ViewBackground->setModel(
					Palapeli::TextureHelper::instance());
	setupSolutionAreaComboBox();

	addPage(generalPage, i18n("General settings"))->
				setIcon(QIcon::fromTheme( QLatin1String( "configure" )));
	//setup page "Mouse interaction"
	addPage(m_triggerPage, i18n("Mouse interaction"))->
				setIcon(QIcon::fromTheme( QLatin1String( "input-mouse" )));
	connect(m_triggerPage, SIGNAL(associationsChanged()),
			       SLOT(updateButtons()));
}

bool Palapeli::ConfigDialog::hasChanged()
{
	return m_triggerPage->hasChanged();
}

bool Palapeli::ConfigDialog::isDefault()
{
	return m_triggerPage->isDefault();
}

void Palapeli::ConfigDialog::updateSettings()
{
	//schedule update of TextureHelper (but only after KConfigDialog has written the settings, which might happen after this slot call)
	QTimer::singleShot(0, Palapeli::TextureHelper::instance(), SLOT(readSettings()));
	m_triggerPage->updateSettings();
}

void Palapeli::ConfigDialog::updateWidgets()
{
	m_triggerPage->updateWidgets();
}

void Palapeli::ConfigDialog::updateWidgetsDefault()
{
	m_triggerPage->updateWidgetsDefault();
}

void Palapeli::ConfigDialog::showEvent(QShowEvent* event)
{
	KConfigDialog::showEvent(event);
	//the dialog is usually created a bit small
	if (!m_shownForFirstTime)
	{
		resize(minimumSize().expandedTo(geometry().size()) + QSize(50, 100));
		m_shownForFirstTime = true;
	}
}

void Palapeli::ConfigDialog::setupSolutionAreaComboBox()
{
	QComboBox* b = m_generalUi.kcfg_SolutionArea;
	b->insertItem(Center,      i18n("Center"),       Center);
	b->insertItem(None,        i18n("None"),         None);
	b->insertItem(TopLeft,     i18n("Top Left"),     TopLeft);
	b->insertItem(TopRight,    i18n("Top Right"),    TopRight);
	b->insertItem(BottomLeft,  i18n("Bottom Left"),  BottomLeft);
	b->insertItem(BottomRight, i18n("Bottom Right"), BottomRight);
	b->setCurrentIndex(Settings::solutionArea());
	connect(b,    SIGNAL(currentIndexChanged(int)),
		this, SLOT(solutionAreaChange(int)));
}

void Palapeli::ConfigDialog::solutionAreaChange(int index)
{
	// Play safe by providing this slot. KConfigSkeleton seems to save the
	// index setting if .kcfg says "Int", but it does not officially support
	// the QComboBox type.
	Settings::setSolutionArea(index);
	Settings::self()->writeConfig();
}

//END Palapeli::ConfigDialog

#include "configdialog.moc"
//#include "configdialog_p.moc"

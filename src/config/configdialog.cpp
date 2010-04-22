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
#include "../engine/texturehelper.h"
#include "../engine/view.h"
#include "settings.h"

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

Palapeli::ConfigDialog::ConfigDialog(Palapeli::View* view, QWidget* parent)
	: KConfigDialog(parent, QString(), Settings::self())
	, m_view(view)
{
	//setup page "General settings"
	QWidget* generalPage = new QWidget;
	m_generalUi.setupUi(generalPage);
	m_generalUi.kcfg_ViewBackground->setModel(view->textureHelper());
	addPage(generalPage, i18n("General settings"))->setIcon(KIcon("configure"));
	//TODO: add TriggerConfigWidget
}

void Palapeli::ConfigDialog::updateSettings()
{
	//schedule update of TextureHelper (but only after KConfigDialog has written the settings, which might happen after this slot call)
	QTimer::singleShot(0, m_generalUi.kcfg_ViewBackground->model(), SLOT(readSettings()));
	//TODO: Here goes code for applying the settings from the TriggerConfigWidget.
}

void Palapeli::ConfigDialog::updateWidgets()
{
	//TODO: Here goes code for initializing the TriggerConfigWidget.
}

void Palapeli::ConfigDialog::updateWidgetsDefault()
{
	//TODO: Here goes code for resetting the TriggerConfigWidget to default values.
}

//END Palapeli::ConfigDialog

#include "configdialog.moc"
#include "configdialog_p.moc"

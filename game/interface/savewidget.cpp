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

#include "savewidget.h"
#include "interfacemanager.h"
#include "../manager.h"

#include <KActionCollection>
#include <KLineEdit>
#include <KLocalizedString>
#include <KStandardGuiItem>
#include <KStandardShortcut>

//BEGIN Palapeli::SaveWidget

Palapeli::SaveWidget* Palapeli::SaveWidget::create(const QString& gameName, Palapeli::AutoscalingItem* parent)
{
	return new Palapeli::SaveWidget(new KLineEdit(gameName), parent);
}

Palapeli::SaveWidget::SaveWidget(KLineEdit* edit, Palapeli::AutoscalingItem* parent)
	: Palapeli::OnScreenDialog(edit, QList<KGuiItem>() << KStandardGuiItem::save() << KStandardGuiItem::cancel(), i18n("Enter a name"), parent)
	, m_edit(edit)
{
	connect(m_edit, SIGNAL(returnPressed()), this, SLOT(handleReturnPressed()));
	connect(this, SIGNAL(buttonPressed(int)), this, SLOT(handleButton(int)));
}

void Palapeli::SaveWidget::handleReturnPressed()
{
	if (!m_edit->text().isEmpty())
		ppMgr()->saveGame(m_edit->text());
	ppIMgr()->hide(Palapeli::InterfaceManager::SaveWidget);
}

void Palapeli::SaveWidget::handleButton(int id)
{
	if (id == 0) // save (the other option, cancel, does nothing)
		handleReturnPressed();
	else
		ppIMgr()->hide(Palapeli::InterfaceManager::SaveWidget);
}

//END Palapeli::SaveWidget

//BEGIN Palapeli::SaveWidgetAction

Palapeli::SaveWidgetAction::SaveWidgetAction(QObject* parent)
	: KAction(KIcon("document-save"), i18n("&Save"), parent)
{
/**/	setEnabled(false);
	setObjectName("palapeli_save");
	setShortcut(KStandardShortcut::shortcut(KStandardShortcut::Save));
	setToolTip(i18n("Save the current game"));

	connect(this, SIGNAL(triggered(bool)), this, SLOT(trigger()));
	connect(ppMgr(), SIGNAL(gameNameChanged(const QString&)), this, SLOT(setGameName(const QString&)));

	KActionCollection* collection = qobject_cast<KActionCollection*>(parent);
	if (collection)
		collection->addAction(objectName(), this);
}

void Palapeli::SaveWidgetAction::setGameName(const QString& name)
{
	m_name = name;
	setEnabled(true); //a game is running now
}

void Palapeli::SaveWidgetAction::trigger()
{
	ppIMgr()->show(Palapeli::InterfaceManager::SaveWidget, QVariantList() << m_name);
}

//END Palapeli::SaveWidget

#include "savewidget.moc"

/***************************************************************************
 *   Copyright (C) 2008 Felix Lemke <lemke.felix@ages-skripte.org>
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

#include "saveaction.h"
#include "manager.h"

#include <KLocalizedString>
#include <KLineEdit>
#include <KMenu>

Palapeli::SaveAction::SaveAction(Manager* manager, QObject* parent)
	: KActionMenu(KIcon("document-save"), i18n("Save"), parent)
	, m_manager(manager)
	, m_menu(new KMenu)
	, m_nameInput(new KLineEdit)
	, m_nameInputAct(new KAction(i18n("Game name:"), m_menu))
	, m_saveAct(new KAction(KIcon("document-save"), i18n("Save game"), m_menu))
{
	setEnabled(false); //do not enable until a game has been started or loaded
	setDelayed(false);
	setStickyMenu(true);
	setToolTip(i18n("Save the current game"));
	connect(m_manager, SIGNAL(gameLoaded(const QString&)), this, SLOT(setPredefinedName(const QString&)));
	//setup actions in menu
	m_nameInputAct->setDefaultWidget(m_nameInput);
	connect(m_nameInput, SIGNAL(returnPressed()), this, SLOT(save()));
	connect(m_saveAct, SIGNAL(triggered()), this, SLOT(save()));
	//setup menu
	m_menu->addTitle(i18n("Enter a name"));
	m_menu->addAction(m_nameInputAct);
	m_menu->addAction(m_saveAct);
	setMenu(m_menu);
}

Palapeli::SaveAction::~SaveAction()
{
	delete m_menu;
	//delete m_nameInputAct; //causes a crash
}

void Palapeli::SaveAction::setPredefinedName(const QString& name)
{
	m_nameInput->setText(name);
	setEnabled(true);
}

void Palapeli::SaveAction::save()
{
	QString name = m_nameInput->text();
	if (!name.isEmpty())
		m_manager->saveGame(m_nameInput->text());
	m_menu->hide();
}

#include "saveaction.moc"

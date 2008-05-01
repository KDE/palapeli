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

#include "loadaction.h"
#include "manager.h"

#include <QSignalMapper>
#include <KLocalizedString>

Palapeli::LoadAction::LoadAction(Palapeli::Manager *manager, QObject *parent)
	: KActionMenu(KIcon("document-open"), i18n("Load"), parent)
	, m_manager(manager)
	, m_mapper(new QSignalMapper(this))
{
	setDelayed(false);
	setStickyMenu(true);
	setToolTip(i18n("Load a saved game"));
	//setup game list
	update();
	connect(m_manager, SIGNAL(saveGameListUpdated()), this, SLOT(update()));
	connect(m_mapper, SIGNAL(mapped(const QString &)), m_manager, SLOT(loadGame(const QString &)));
}

Palapeli::LoadAction::~LoadAction()
{
	delete m_mapper;
	QHashIterator<QString, KAction *> iterActions(m_actions);
	while (iterActions.hasNext())
		delete iterActions.next().value();
}

void Palapeli::LoadAction::update()
{
	const QList<QString> games = m_manager->availableSaveGames();
	const QList<QString> actions = m_actions.keys();
	//add new savegames
	foreach (QString name, games)
	{
		if (actions.contains(name))
			continue;
		KAction *newAct = new KAction(name, this);
		connect(newAct, SIGNAL(triggered(bool)), m_mapper, SLOT(map()));
		m_mapper->setMapping(newAct, name);
		addAction(newAct);
		m_actions[name] = newAct;
	}
	//remove old savegames
	foreach (QString name, actions)
	{
		if (games.contains(name))
			continue;
		KAction *oldAct = m_actions.take(name);
		m_mapper->removeMappings(oldAct);
		removeAction(oldAct);
		delete oldAct;
	}
	//disable action if nothing can be loaded
	setEnabled(m_actions.count() > 0);
}

#include "loadaction.moc"

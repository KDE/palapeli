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

#include "statemanager.h"

#include <QCoreApplication>
#include <KConfig>
#include <KConfigGroup>
#include <KDebug>
#include <KStandardDirs>

Palapeli::StateManager::StateManager()
	: m_isPersistent(false) //startup counts as non-persistent
	, m_id(QCoreApplication::applicationPid())
	, m_config(new KConfig(KStandardDirs::locateLocal("config", "palapeli-staterc"), KConfig::SimpleConfig))
	, m_configGroup(new KConfigGroup(m_config, QString::number(m_id)))
{
	kDebug() << KStandardDirs::locateLocal("config", "palapeli-staterc");
}

Palapeli::StateManager::~StateManager()
{
	delete m_configGroup;
	delete m_config;
}

QString Palapeli::StateManager::gameName() const
{
	return m_gameName;
}

int Palapeli::StateManager::id() const
{
	return m_id;
}

bool Palapeli::StateManager::isPersistent() const
{
	return m_isPersistent;
}

void Palapeli::StateManager::setPersistent(bool isPersistent)
{
	if (m_isPersistent == isPersistent)
		return;
	m_isPersistent = isPersistent;
	m_configGroup->writeEntry("Persistent", isPersistent);
	m_configGroup->sync();
}

void Palapeli::StateManager::setGameName(const QString& name)
{
	if (m_gameName == name)
		return;
	m_gameName = name;
	m_configGroup->writeEntry("GameName", name);
	m_configGroup->sync();
}

#include "statemanager.moc"

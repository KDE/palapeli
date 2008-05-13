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

#include "savegamemodel.h"
#include "manager.h"

Palapeli::SavegameModel::SavegameModel(Manager* manager)
	: QStringListModel()
	, m_manager(manager)
{
	connect(m_manager, SIGNAL(savegameCreated(const QString&)), this, SLOT(savegameCreated(const QString&)));
	connect(m_manager, SIGNAL(savegameDeleted(const QString&)), this, SLOT(savegameDeleted(const QString&)));
}

Palapeli::SavegameModel::~SavegameModel()
{
}

void Palapeli::SavegameModel::savegameCreated(const QString& name)
{
	const QStringList games = stringList();
	if (!games.contains(name))
	{
		//insert new game at the end of the list
		insertRows(games.count(), 1);
		setData(createIndex(games.count(), 0), name, Qt::DisplayRole);
	}
}

void Palapeli::SavegameModel::savegameDeleted(const QString& name)
{
	const QStringList games = stringList();
	if (games.contains(name))
		removeRows(games.indexOf(name), 1);
}

#include "savegamemodel.moc"

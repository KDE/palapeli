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

Palapeli::SavegameModel::SavegameModel(const QStringList& list)
	: QStringListModel(list)
{
}

bool Palapeli::SavegameModel::lessThan(const QString& s1, const QString& s2)
{
	return QString::localeAwareCompare(s1, s2) < 0;
}

void Palapeli::SavegameModel::savegameCreated(const QString& name)
{
	const QStringList games = stringList();
	if (games.contains(name))
		return;
	//find right position to insert the new item (lexical order should be preserved)
	int position = 0;
	for (; position < games.count(); ++position)
	{
		if (Palapeli::SavegameModel::lessThan(name, games.at(position)))
			break;
	}
	//insert new item
	insertRows(position, 1);
	setData(createIndex(position, 0), name, Qt::DisplayRole);
}

void Palapeli::SavegameModel::savegameDeleted(const QString& name)
{
	const int position = stringList().indexOf(name);
	if (position > -1)
		removeRows(position, 1);
}

#include "savegamemodel.moc"

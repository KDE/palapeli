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

#include "savegameview.h"
#include "savegameview_p.h"
#include "manager.h"

#include <QGridLayout>
#include <QListView>

Palapeli::SavegameView::SavegameView(Manager* manager, QWidget* parent)
	: QWidget(parent)
	, m_layout(new QGridLayout)
	, m_manager(manager)
	, m_model(new Palapeli::SavegameModel(manager))
	, m_view(new QListView)
{
	m_view->setModel(m_model);
	m_layout->addWidget(m_view, 0, 0);
	setLayout(m_layout);
}

Palapeli::SavegameView::~SavegameView()
{
	delete m_view;
	delete m_model;
	delete m_layout;
}

Palapeli::SavegameModel::SavegameModel(Manager* manager)
	: QAbstractListModel()
	, m_manager(manager)
	, m_saveGames(m_manager->availableSaveGames())
{
}

Palapeli::SavegameModel::~SavegameModel()
{
}

QVariant Palapeli::SavegameModel::data(const QModelIndex &index, int role) const
{
	int row = index.row();
	if (row < 0 || row >= m_saveGames.count())
		return QVariant();
	switch (role)
	{
		case Qt::DisplayRole:
			return m_saveGames.at(row);
		default:
			return QVariant();
	}
}

QVariant Palapeli::SavegameModel::headerData(int /*index*/, Qt::Orientation /*orientation*/, int /*role*/) const
{
	return QVariant();
}

int Palapeli::SavegameModel::rowCount(const QModelIndex &parent) const
{
	return parent.isValid() ? 0 : m_saveGames.count();
}

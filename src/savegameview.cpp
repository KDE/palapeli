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
#include "savegamemodel.h"
#include "manager.h"

#include <QListView>
#include <KAction>
#include <KIcon>
#include <KLocalizedString>
#include <KToolBar>

Palapeli::SavegameView::SavegameView(Manager* manager, QWidget* parent)
	: KMainWindow(parent)
	, m_manager(manager)
	, m_model(new Palapeli::SavegameModel(manager))
	, m_view(new QListView)
	, m_loadAct(new KAction(KIcon("document-open"), i18nc("the verb, as in 'Load game'", "Load"), this))
	, m_deleteAct(new KAction(KIcon("edit-delete"), i18n("Delete"), this))
{
	//fill toolbar
	KToolBar *mainToolBar = toolBar("savegamesToolBar");
	mainToolBar->addAction(m_loadAct);
	m_loadAct->setEnabled(false); //nothing selected to load by now
	connect(m_loadAct, SIGNAL(triggered()), this, SLOT(loadSelected()));
	mainToolBar->addAction(m_deleteAct);
	m_deleteAct->setEnabled(false); //like above
	connect(m_deleteAct, SIGNAL(triggered()), this, SLOT(deleteSelected()));
	//list view
	m_view->setModel(m_model);
	m_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
	setCentralWidget(m_view);
	connect(m_view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(selectionChanged()));
}

Palapeli::SavegameView::~SavegameView()
{
	delete m_loadAct;
	delete m_deleteAct;
	delete m_view;
	delete m_model;
}

void Palapeli::SavegameView::deleteSelected()
{
	//gather list of games to delete
	QList<QString> names;
	foreach (QModelIndex item, m_view->selectionModel()->selectedIndexes())
		names << m_model->data(item, Qt::DisplayRole).toString();
	//delete games
	foreach (QString name, names)
		m_manager->deleteGame(name);
}

void Palapeli::SavegameView::loadSelected()
{
	//get name of first selected game and load this one
	QModelIndex selected = m_view->selectionModel()->currentIndex();
	if (!selected.isValid())
		return;
	m_manager->loadGame(m_model->data(selected, Qt::DisplayRole).toString());
}

void Palapeli::SavegameView::selectionChanged()
{
	int selectedCount = m_view->selectionModel()->selectedIndexes().count();
	m_deleteAct->setEnabled(selectedCount > 0 && false); //disabled at the moment because of missing implementation in Manager
	m_loadAct->setEnabled(selectedCount == 1);
}

#include "savegameview.moc"

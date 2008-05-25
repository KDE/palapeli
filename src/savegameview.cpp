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
#include "gamestorage/gamestorageattribs.h"
#include "gamestorage/gamestorage.h"
#include "gamestorage/gamestorageitem.h"
#include "manager.h"
#include "mainwindow.h"
#include "savegamemodel.h"

#include <QListView>
#include <KAction>
#include <KIcon>
#include <KFileDialog>
#include <KLocalizedString>
#include <KToolBar>

Palapeli::SavegameView::SavegameView(Manager* manager, QWidget* parent)
	: KMainWindow(parent)
	, m_manager(manager)
	, m_model(new Palapeli::SavegameModel(manager))
	, m_view(new QListView)
	, m_loadAct(new KAction(KIcon("document-open"), i18nc("the verb, as in 'Load game'", "Load"), this))
	, m_deleteAct(new KAction(KIcon("edit-delete"), i18n("Delete"), this))
	, m_importAct(new KAction(KIcon("document-import"), i18nc("the verb, as in 'Import games'", "Import"), this))
	, m_exportAct(new KAction(KIcon("document-export"), i18nc("the verb, as in 'Export games'", "Export"), this))
{
	//fill toolbar
	KToolBar *mainToolBar = toolBar("savegamesToolBar");
	mainToolBar->addAction(m_loadAct);
	m_loadAct->setEnabled(false); //nothing selected to load by now
	connect(m_loadAct, SIGNAL(triggered()), this, SLOT(loadSelected()));
	mainToolBar->addAction(m_importAct);
	m_importAct->setEnabled(true); //always active
	connect(m_importAct, SIGNAL(triggered()), this, SLOT(importSelected()));
	mainToolBar->addAction(m_exportAct);
	m_exportAct->setEnabled(false); //like for loadAct
	connect(m_exportAct, SIGNAL(triggered()), this, SLOT(exportSelected()));
	mainToolBar->addAction(m_deleteAct);
	m_deleteAct->setEnabled(false); //like for loadAct
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
	delete m_importAct;
	delete m_exportAct;
	delete m_view;
	delete m_model;
}

void Palapeli::SavegameView::deleteSelected()
{
	//gather list of games to delete
	QList<QString> names;
	foreach (const QModelIndex& item, m_view->selectionModel()->selectedIndexes())
		names << m_model->data(item, Qt::DisplayRole).toString();
	//delete games
	foreach (const QString& name, names)
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

void Palapeli::SavegameView::importSelected()
{
	QString target = KFileDialog::getOpenFileName(KUrl("kfiledialog:///palapeli"), "*.psga|" + i18nc("Used as filter description in a file dialog.", "Palapeli Saved Game Archive (*.psga)"), m_manager->window(), i18nc("Used as caption for file dialog.", "Select archive to import games from - Palapeli"));
	if (target.isEmpty()) //process aborted by user
		return;
	Palapeli::GameStorage gs;
	Palapeli::GameStorageItems importedItems = gs.importItems(KUrl(target));
	foreach (const Palapeli::GameStorageItem& item, importedItems)
	{
		if (item.type() == Palapeli::GameStorageItem::SavedGame)
			m_manager->savegameWasCreated(item.metaData());
	}
}

void Palapeli::SavegameView::exportSelected()
{
	Palapeli::GameStorage gs;
	//gather a list of all game items to export
	Palapeli::GameStorageItems exportableItems;
	foreach (const QModelIndex& index, m_view->selectionModel()->selectedIndexes())
	{
		QString gameName = m_model->data(index, Qt::DisplayRole).toString();
		exportableItems += gs.queryItems(Palapeli::GameStorageAttributes() << new Palapeli::GameStorageMetaAttribute(gameName) << new Palapeli::GameStorageTypeAttribute(Palapeli::GameStorageItem::SavedGame));
	}
	if (exportableItems.count() == 0)
		return;
	//ask the user for a file name
	QString target = KFileDialog::getSaveFileName(KUrl("kfiledialog:///palapeli"), "*.psga|" + i18nc("Used as filter description in a file dialog.", "Palapeli Saved Game Archive (*.psga)"), m_manager->window(), i18nc("Used as caption for file dialog.", "Select file to export selected games to - Palapeli"));
	if (target.isEmpty()) //process aborted by user
		return;
	gs.exportItems(KUrl(target), exportableItems);
}

void Palapeli::SavegameView::selectionChanged()
{
	int selectedCount = m_view->selectionModel()->selectedIndexes().count();
	m_deleteAct->setEnabled(selectedCount > 0);
	m_exportAct->setEnabled(selectedCount > 0);
	m_loadAct->setEnabled(selectedCount == 1);
}

#include "savegameview.moc"

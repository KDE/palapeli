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
#include "../storage/gamestorageattribs.h"
#include "../storage/gamestorage.h"
#include "../storage/gamestorageitem.h"
#include "manager.h"
#include "mainwindow.h"
#include "savegamemodel.h"

#include <QListView>
#include <KAction>
#include <KIcon>
#include <KFileDialog>
#include <KLocalizedString>
#include <KToolBar>

Palapeli::SavegameView::SavegameView(QWidget* parent)
	: KMainWindow(parent)
	, m_view(new QListView)
	, m_loadAct(new KAction(KIcon("document-open"), i18nc("the verb, as in 'Load game'", "Load"), this))
	, m_deleteAct(new KAction(KIcon("edit-delete"), i18n("Delete"), this))
	, m_importAct(new KAction(KIcon("document-import"), i18nc("the verb, as in 'Import games'", "Import"), this))
	, m_exportAct(new KAction(KIcon("document-export"), i18nc("the verb, as in 'Export games'", "Export"), this))
{
	connect(ppMgr(), SIGNAL(interactionModeChanged(bool)), this, SLOT(changeInteractionMode(bool)));
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
	m_view->setModel(ppMgr()->savegameModel());
	m_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_view->setEditTriggers(QAbstractItemView::NoEditTriggers); //disable editing as the there is no rename function available in the model
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
}

void Palapeli::SavegameView::deleteSelected()
{
	//gather list of games to delete
	QList<QString> names;
	foreach (const QModelIndex& item, m_view->selectionModel()->selectedIndexes())
		names << ppMgr()->savegameModel()->data(item, Qt::DisplayRole).toString();
	//delete games
	foreach (const QString& name, names)
		ppMgr()->deleteGame(name);
}

void Palapeli::SavegameView::loadSelected()
{
	//get name of first selected game and load this one
	QModelIndex selected = m_view->selectionModel()->currentIndex();
	if (!selected.isValid())
		return;
	ppMgr()->loadGame(ppMgr()->savegameModel()->data(selected, Qt::DisplayRole).toString());
}

void Palapeli::SavegameView::importSelected()
{
	//ask the user for a target
	KUrl target = KFileDialog::getOpenUrl(KUrl("kfiledialog:///palapeli"), "*.psga|" + i18nc("Used as filter description in a file dialog.", "Palapeli Saved Game Archive (*.psga)"), ppMgr()->window(), i18nc("Used as caption for file dialog.", "Select archive to import games from - Palapeli"));
	if (target.isEmpty()) //process aborted by user
		return;
	Palapeli::GameStorage gs;
	Palapeli::GameStorageItems importedItems = gs.importItems(target);
	foreach (const Palapeli::GameStorageItem& item, importedItems)
	{
		if (item.type() == Palapeli::GameStorageItem::SavedGame)
			ppMgr()->savegameWasCreated(item.metaData());
	}
}

void Palapeli::SavegameView::exportSelected()
{
	Palapeli::GameStorage gs;
	//gather a list of all game items to export
	Palapeli::GameStorageItems exportableItems;
	foreach (const QModelIndex& index, m_view->selectionModel()->selectedIndexes())
	{
		QString gameName = ppMgr()->savegameModel()->data(index, Qt::DisplayRole).toString();
		exportableItems += gs.queryItems(Palapeli::GameStorageAttributes() << new Palapeli::GameStorageMetaAttribute(gameName) << new Palapeli::GameStorageTypeAttribute(Palapeli::GameStorageItem::SavedGame));
	}
	if (exportableItems.count() == 0)
		return;
	//ask the user for a file name
	KUrl target = KFileDialog::getSaveUrl(KUrl("kfiledialog:///palapeli"), "*.psga|" + i18nc("Used as filter description in a file dialog.", "Palapeli Saved Game Archive (*.psga)"), ppMgr()->window(), i18nc("Used as caption for file dialog.", "Select file to export selected games to - Palapeli"));
	if (target.isEmpty()) //process aborted by user
		return;
	gs.exportItems(target, exportableItems);
}

void Palapeli::SavegameView::selectionChanged()
{
	int selectedCount = m_view->selectionModel()->selectedIndexes().count();
	m_deleteAct->setEnabled(selectedCount > 0);
	m_exportAct->setEnabled(selectedCount > 0);
	m_loadAct->setEnabled(selectedCount == 1);
}

void Palapeli::SavegameView::changeInteractionMode(bool allowGameInteraction)
{
	m_importAct->setEnabled(allowGameInteraction);
	if (allowGameInteraction)
		selectionChanged(); //this function knows better when the other actions can be enabled
	else
	{
		m_deleteAct->setEnabled(false);
		m_exportAct->setEnabled(false);
		m_loadAct->setEnabled(false);
	}
}

#include "savegameview.moc"

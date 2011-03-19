/***************************************************************************
 *   Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
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

#include "collectionwidget.h"
#include "../file-io/collection-delegate.h"
#include "../file-io/collection-filesystem.h"
#include "../file-io/collection-list.h"
#include "../file-io/collection-view.h"
#include "../file-io/puzzle.h"

#include <KAction>
#include <KActionCollection>
#include <KLocalizedString>
#include <KStandardDirs>
#include <KStandardShortcut>

Palapeli::CollectionWidget::CollectionWidget()
	: Palapeli::TabWindow(QLatin1String("palapeli-collection"))
	, m_view(new Palapeli::CollectionView)
	, m_localCollection(new Palapeli::LocalCollection)
	, m_fsCollection(new Palapeli::FileSystemCollection)
{
	//setup view
	m_view->setModel(m_localCollection);
	connect(m_view, SIGNAL(playRequest(const QModelIndex&)), this, SIGNAL(playRequest(const QModelIndex&)));
	connect(m_view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(handleSelectionChanged()));
	//setup actions
	KAction* createAct = new KAction(KIcon("tools-wizard"), i18n("Create &new puzzle..."), 0); //FIXME: This should be a custom "actions/puzzle-new" icon.
	createAct->setShortcut(KStandardShortcut::openNew());
	createAct->setToolTip(i18n("Create a new puzzle using an image file from your disk"));
	actionCollection()->addAction("file_new", createAct);
	connect(createAct, SIGNAL(triggered()), this, SIGNAL(createRequest()));
	KAction* importAct = new KAction(KIcon("document-import"), i18n("&Import from file..."), 0);
	importAct->setToolTip(i18n("Import a new puzzle from a file into your collection"));
	actionCollection()->addAction("file_import", importAct);
	connect(importAct, SIGNAL(triggered()), this, SLOT(handleImportRequest()));
	m_exportAct = new KAction(KIcon("document-export"), i18n("&Export to file..."), 0);
	m_exportAct->setEnabled(false); //will be enabled when something is selected
	m_exportAct->setToolTip(i18n("Export the selected puzzle from your collection into a file"));
	actionCollection()->addAction("file_export", m_exportAct);
	connect(m_exportAct, SIGNAL(triggered()), this, SLOT(handleExportRequest()));
	m_deleteAct = new KAction(KIcon("archive-remove"), i18n("&Delete"), 0);
	m_deleteAct->setEnabled(false); //will be enabled when something is selected
	m_deleteAct->setToolTip(i18n("Delete the selected puzzle from your collection"));
	actionCollection()->addAction("file_delete", m_deleteAct);
	connect(m_deleteAct, SIGNAL(triggered()), this, SLOT(handleDeleteRequest()));
	//setup GUI
	setupGUI();
	setCentralWidget(m_view);
}

void Palapeli::CollectionWidget::startPuzzle(const KUrl& url)
{
	QModelIndex index = m_fsCollection->providePuzzle(url);
	emit playRequest(index);
}

QModelIndex Palapeli::CollectionWidget::storeGeneratedPuzzle(Palapeli::Puzzle* puzzle)
{
	return m_localCollection->storeGeneratedPuzzle(puzzle);
}

void Palapeli::CollectionWidget::handleDeleteRequest()
{
	QModelIndexList indexes = m_view->selectionModel()->selectedIndexes();
	foreach (const QModelIndex& index, indexes)
		m_localCollection->deletePuzzle(index);
}

void Palapeli::CollectionWidget::handleExportRequest()
{
	QModelIndexList indexes = m_view->selectionModel()->selectedIndexes();
	foreach (const QModelIndex& index, indexes)
	{
		QObject* puzzlePayload = index.data(Palapeli::Collection::PuzzleObjectRole).value<QObject*>();
		Palapeli::Puzzle* puzzle = qobject_cast<Palapeli::Puzzle*>(puzzlePayload);
		if (!puzzle)
			continue;
		m_fsCollection->importPuzzle(puzzle);
	}
}

void Palapeli::CollectionWidget::handleImportRequest()
{
	QModelIndexList selectedPuzzles = m_fsCollection->selectPuzzles();
	foreach (const QModelIndex& index, selectedPuzzles)
	{
		QObject* puzzlePayload = index.data(Palapeli::Collection::PuzzleObjectRole).value<QObject*>();
		Palapeli::Puzzle* puzzle = qobject_cast<Palapeli::Puzzle*>(puzzlePayload);
		if (!puzzle)
			continue;
		m_localCollection->importPuzzle(puzzle);
	}
}

void Palapeli::CollectionWidget::handleSelectionChanged()
{
	const QModelIndexList indexes = m_view->selectionModel()->selectedIndexes();
	bool enableDeleteAct = !indexes.isEmpty();
	foreach (const QModelIndex& index, indexes)
		if (index.data(Palapeli::Collection::IsDeleteableRole) == QVariant(false))
		{
			enableDeleteAct = false;
			break;
		}
	m_exportAct->setEnabled(!indexes.isEmpty());
	m_deleteAct->setEnabled(enableDeleteAct);
}

#include "collectionwidget.moc"

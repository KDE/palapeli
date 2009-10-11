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

#include "librarywidget.h"
#include "../file-io/collection-filesystem.h"
#include "../file-io/collection-list.h"
#include "../file-io/librarydelegate.h"
#include "../file-io/puzzle.h"

#include <QListView>
#include <QStandardItemModel>
#include <KAction>
#include <KActionCollection>
#include <KLocalizedString>
#include <KStandardDirs>

Palapeli::LibraryWidget::LibraryWidget()
	: Palapeli::TabWindow(QLatin1String("palapeli-library"))
	, m_view(new QListView)
	, m_model(new Palapeli::LibraryCollection)
	, m_fsCollection(new Palapeli::FileSystemCollection)
{
	//setup view
	m_view->setModel(m_model);
	m_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
	Palapeli::LibraryDelegate* delegate = new Palapeli::LibraryDelegate(m_view);
	connect(delegate, SIGNAL(playRequest(const QModelIndex&)), this, SIGNAL(playRequest(const QModelIndex&)));
	connect(m_view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(handleSelectionChanged()));
	//setup actions
	KAction* importAct = new KAction(KIcon("document-import"), i18n("&Import..."), 0);
	importAct->setToolTip(i18n("Import a new puzzle from a file"));
	actionCollection()->addAction("file_import", importAct);
	connect(importAct, SIGNAL(triggered()), this, SLOT(handleImportRequest()));
	m_exportAct = new KAction(KIcon("document-export"), i18n("&Export..."), 0);
	m_exportAct->setToolTip(i18n("Export the selected puzzle from the library into a file"));
	actionCollection()->addAction("file_export", m_exportAct);
	connect(m_exportAct, SIGNAL(triggered()), this, SLOT(handleExportRequest()));
	m_deleteAct = new KAction(KIcon("archive-remove"), i18n("&Delete"), 0);
	m_deleteAct->setEnabled(false); //will be enabled when something is selected
	m_deleteAct->setToolTip(i18n("Delete the selected puzzle from the library"));
	actionCollection()->addAction("file_delete", m_deleteAct);
	connect(m_deleteAct, SIGNAL(triggered()), this, SLOT(handleDeleteRequest()));
	//setup GUI
	setupGUI();
	setCentralWidget(m_view);
}

void Palapeli::LibraryWidget::resizeEvent(QResizeEvent* event)
{
	Q_UNUSED(event)
	//HACK: The KWidgetItemDelegate does not update its size hints after the view has resized.
	QStandardItemModel emptyDummyModel;
	m_view->setModel(&emptyDummyModel); //Using 0 here gives useless warnings.
	m_view->setModel(m_model);
	connect(m_view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(handleSelectionChanged()));
}

void Palapeli::LibraryWidget::handleDeleteRequest()
{
#if 0
	m_model->deletePuzzle(m_view->selectionModel()->selectedIndexes());
#endif
}

void Palapeli::LibraryWidget::handleExportRequest()
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

void Palapeli::LibraryWidget::handleImportRequest()
{
	QModelIndexList selectedPuzzles = m_fsCollection->selectPuzzles();
	foreach (const QModelIndex& index, selectedPuzzles)
	{
		QObject* puzzlePayload = index.data(Palapeli::Collection::PuzzleObjectRole).value<QObject*>();
		Palapeli::Puzzle* puzzle = qobject_cast<Palapeli::Puzzle*>(puzzlePayload);
		if (!puzzle)
			continue;
		m_model->importPuzzle(puzzle);
	}
}

void Palapeli::LibraryWidget::handleSelectionChanged()
{
	const QModelIndexList indexes = m_view->selectionModel()->selectedIndexes();
	bool enableActions = true;
	if (!indexes.isEmpty())
		foreach (const QModelIndex& index, indexes)
			if (index.data(Palapeli::Collection::IsDeleteableRole) == QVariant(true))
			{
				enableActions = false;
				break;
			}
	m_deleteAct->setEnabled(enableActions);
}

#include "librarywidget.moc"

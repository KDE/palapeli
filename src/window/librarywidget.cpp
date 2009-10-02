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
#include "../file-io/librarydelegate.h"
#include "../file-io/librarymodel.h"
#include "../file-io/puzzle.h"

#include <QListView>
#include <QStandardItemModel>
#include <KAction>
#include <KActionCollection>
#include <KFileDialog>
#include <KLocalizedString>

Palapeli::LibraryWidget::LibraryWidget()
	: Palapeli::TabWindow(QLatin1String("palapeli-library"))
	, m_view(new QListView)
	, m_model(new Palapeli::LibraryModel)
{
	//setup view
	m_view->setModel(m_model);
	m_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
	Palapeli::LibraryDelegate* delegate = new Palapeli::LibraryDelegate(m_view);
	connect(delegate, SIGNAL(playRequest(const QString&)), this, SLOT(handlePlayRequest(const QString&)));
	connect(m_view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(handleSelectionChanged()));
	//setup actions
	KAction* importAct = new KAction(KIcon("document-import"), i18n("&Import..."), 0);
	importAct->setToolTip(i18n("Import a new puzzle from a file"));
	actionCollection()->addAction("file_import", importAct);
	connect(importAct, SIGNAL(triggered()), this, SLOT(handleImportRequest()));
	m_exportAct = new KAction(KIcon("document-export"), i18n("&Export..."), 0);
	m_exportAct->setEnabled(false); //not implemented yet
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

Palapeli::LibraryModel* Palapeli::LibraryWidget::model() const
{
	return m_model;
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
	m_model->deletePuzzle(m_view->selectionModel()->selectedIndexes());
}

void Palapeli::LibraryWidget::handleExportRequest()
{
	QModelIndexList indexes = m_view->selectionModel()->selectedIndexes();
	foreach (const QModelIndex& index, indexes)
	{
		Palapeli::Puzzle* puzzle = m_model->puzzle(index);
		if (!puzzle)
			continue;
		const KUrl startLoc = QString::fromLatin1("kfiledialog:///palapeli-export/%1.pala").arg(puzzle->location().identifier());
		const QString filter = QLatin1String("*.pala|Palapeli puzzles (*.pala)");
		KUrl url = KFileDialog::getSaveUrl(startLoc, filter);
		if (!url.isEmpty())
			m_model->exportPuzzle(index, url);
	}
}

void Palapeli::LibraryWidget::handleImportRequest()
{
	const QString filter = QLatin1String("*.pala|Palapeli puzzles (*.pala)");
	KUrl::List urls = KFileDialog::getOpenUrls(KUrl("kfiledialog:///palapeli-import"), filter);
	foreach (const KUrl& url, urls)
		if (!url.isEmpty())
			m_model->importPuzzle(url);
}

void Palapeli::LibraryWidget::handlePlayRequest(const QString& puzzleIdentifier)
{
	Palapeli::Puzzle* puzzle = m_model->puzzle(puzzleIdentifier);
	if (puzzle)
		emit playRequest(puzzle);
}

void Palapeli::LibraryWidget::handleSelectionChanged()
{
	const QModelIndexList indexes = m_view->selectionModel()->selectedIndexes();
	bool enableActions = false;
	if (!indexes.isEmpty())
		foreach (const QModelIndex& index, indexes)
			if (index.data(Palapeli::LibraryModel::IsDeleteableRole) == QVariant(true))
				enableActions = true;
	m_deleteAct->setEnabled(enableActions);
	m_exportAct->setEnabled(enableActions);
}

#include "librarywidget.moc"

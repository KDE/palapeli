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

#include <QListView>
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

void Palapeli::LibraryWidget::handleDeleteRequest()
{
	m_model->deletePuzzle(m_view->selectionModel()->selectedIndexes());
}

void Palapeli::LibraryWidget::handleImportRequest()
{
	const QString filter = QLatin1String("*.pala|Palapeli puzzles (*.pala)");
	KUrl::List urls = KFileDialog::getOpenUrls(KUrl("kfiledialog:///palapeli-import"), filter);
	foreach (const KUrl& url, urls)
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
	if (indexes.isEmpty())
	{
		m_deleteAct->setEnabled(false);
		m_exportAct->setEnabled(false);
	}
	else
	{
		m_deleteAct->setEnabled(indexes[0].data(Palapeli::LibraryModel::IsDeleteableRole) == QVariant(true));
		m_exportAct->setEnabled(true);
	}
}

#include "librarywidget.moc"

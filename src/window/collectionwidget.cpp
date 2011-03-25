/***************************************************************************
 *   Copyright 2009-2011 Stefan Majewsky <majewsky@gmx.net>
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
#include "../file-io/collection.h"
#include "../file-io/collection-delegate.h"
#include "../file-io/collection-view.h"
#include "../file-io/components.h"
#include "../file-io/puzzle.h"

#include <KDE/KAction>
#include <KDE/KActionCollection>
#include <KDE/KFileDialog>
#include <KDE/KLocalizedString>
#include <KDE/KStandardDirs>
#include <KDE/KStandardShortcut>

Palapeli::CollectionWidget::CollectionWidget()
	: Palapeli::TabWindow(QLatin1String("palapeli-collection"))
	, m_view(new Palapeli::CollectionView)
{
	//setup view
	m_view->setModel(Palapeli::Collection::instance());
	connect(m_view, SIGNAL(playRequest(Palapeli::Puzzle*)), this, SIGNAL(playRequest(Palapeli::Puzzle*)));
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

void Palapeli::CollectionWidget::startPuzzle(const QString& path)
{
	//create puzzle instance
	const QString id = Palapeli::Puzzle::fsIdentifier(path);
	emit playRequest(new Palapeli::Puzzle(new Palapeli::ArchiveStorageComponent, path, id));
}

void Palapeli::CollectionWidget::handleDeleteRequest()
{
	QModelIndexList indexes = m_view->selectionModel()->selectedIndexes();
	foreach (const QModelIndex& index, indexes)
		Palapeli::Collection::instance()->deletePuzzle(index);
}

void Palapeli::CollectionWidget::handleExportRequest()
{
	QModelIndexList indexes = m_view->selectionModel()->selectedIndexes();
	Palapeli::Collection* coll = Palapeli::Collection::instance();
	foreach (const QModelIndex& index, indexes)
	{
		Palapeli::Puzzle* puzzle = coll->puzzleFromIndex(index);
		if (!puzzle)
			continue;
		//get puzzle name (as an initial guess for the file name)
		puzzle->get(Palapeli::PuzzleComponent::Metadata).waitForFinished();
		const Palapeli::MetadataComponent* cmp = puzzle->component<Palapeli::MetadataComponent>();
		if (!cmp)
			continue;
		//ask user for target file name
		const QString startLoc = QString::fromLatin1("kfiledialog:///palapeli-export/%1.puzzle").arg(cmp->metadata.name);
		const QString filter = i18nc("Filter for a file dialog", "*.puzzle|Palapeli puzzles (*.puzzle)");
		const QString location = KFileDialog::getSaveFileName(KUrl(startLoc), filter);
		if (location.isEmpty())
			continue; //process aborted by user
		//do export
		coll->exportPuzzle(index, location);
	}
}

void Palapeli::CollectionWidget::handleImportRequest()
{
	const QString filter = i18nc("Filter for a file dialog", "*.puzzle|Palapeli puzzles (*.puzzle)");
	const QStringList paths = KFileDialog::getOpenFileNames(KUrl("kfiledialog:///palapeli-import"), filter);
	Palapeli::Collection* coll = Palapeli::Collection::instance();
	foreach (const QString& path, paths)
		coll->importPuzzle(path);
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

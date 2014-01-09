/***************************************************************************
 *   Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
 *   Copyright 2014 Ian Wadham <iandw.au@gmail.com>
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

#include "gameplay.h"

#include "../file-io/collection-view.h"
#include "../window/puzzletablewidget.h"
#include "puzzlepreview.h"

#include "scene.h"
#include "view.h"
#include "../file-io/puzzle.h"
#include "../file-io/components.h"
#include "../file-io/collection.h"
#include "../creator/puzzlecreator.h"

#include "../config/configdialog.h"
#include "settings.h"

#include <QtGui/QStackedWidget>
#include <QPointer>
#include <KDE/KAction>
#include <KDE/KActionCollection>
#include <KDE/KLocalizedString>
#include <KDE/KMessageBox>
#include <KDE/KFileDialog>

//TODO: move LoadingWidget into here (stack into m_centralWidget)

Palapeli::GamePlay::GamePlay(MainWindow* mainWindow)
	: QObject(mainWindow)
	, m_mainWindow(mainWindow)
	, m_centralWidget(new QStackedWidget)
	, m_collectionView(new Palapeli::CollectionView)
	, m_puzzleTable(new Palapeli::PuzzleTableWidget)
	, m_puzzlePreview(0)
{
}

Palapeli::GamePlay::~GamePlay()
{
	delete m_puzzlePreview;
}

void Palapeli::GamePlay::init()
{
	// Set up the collection view.
	m_collectionView->setModel(Palapeli::Collection::instance());
	connect(m_collectionView, SIGNAL(playRequest(Palapeli::Puzzle*)), SLOT(playPuzzle(Palapeli::Puzzle*)));

	// Set up the puzzle table.
	m_puzzleTable->showStatusBar(Settings::showStatusBar());

	// Set up the central widget.
	m_centralWidget->addWidget(m_collectionView);
	m_centralWidget->addWidget(m_puzzleTable);
	m_centralWidget->setCurrentWidget(m_collectionView);
	m_mainWindow->setCentralWidget(m_centralWidget);
}

//BEGIN action handlers

void Palapeli::GamePlay::playPuzzle(Palapeli::Puzzle* puzzle)
{
	if (!puzzle)
		return;
	m_puzzleTable->view()->scene()->loadPuzzle(puzzle);

	m_centralWidget->setCurrentWidget(m_puzzleTable);
	m_mainWindow->actionCollection()->action("go_collection")->setEnabled(true);
	m_mainWindow->actionCollection()->action("game_restart")->setEnabled(true);
	m_puzzlePreview = new Palapeli::PuzzlePreview();
	m_mainWindow->actionCollection()->action("toggle_preview")->setEnabled(true);

	// Get metadata from archive (tar), to be sure of getting image data.
	// The config/palapeli-collectionrc file lacks image metadata (because
	// Palapeli must load the collection-list quickly at startup time).
	const Palapeli::PuzzleComponent* as =
		puzzle->get(Palapeli::PuzzleComponent::ArchiveStorage);
	const Palapeli::PuzzleComponent* cmd = (as == 0) ? 0 :
		as->cast(Palapeli::PuzzleComponent::Metadata);
	if (cmd) {
		// Load puzzle preview image from metadata.
		const Palapeli::PuzzleMetadata md =
			dynamic_cast<const Palapeli::MetadataComponent*>(cmd)->
			metadata;
		m_puzzlePreview->loadImageFrom(md);
		m_mainWindow->setCaption(md.name);	// Set main title.
	}

	m_puzzlePreview->setVisible(Settings::puzzlePreviewVisible());
	connect (m_puzzlePreview, SIGNAL(closing()),
		SLOT(actionTogglePreview()));	// Hide preview: do not delete.
}

void Palapeli::GamePlay::playPuzzleFile(const QString& path)
{
	const QString id = Palapeli::Puzzle::fsIdentifier(path);
	playPuzzle(new Palapeli::Puzzle(new Palapeli::ArchiveStorageComponent,
						path, id));
}

void Palapeli::GamePlay::actionGoCollection()
{
	m_centralWidget->setCurrentWidget(m_collectionView);
	m_mainWindow->actionCollection()->action("go_collection")->setEnabled(false);
	m_mainWindow->actionCollection()->action("game_restart")->setEnabled(false);
	m_mainWindow->actionCollection()->action("toggle_preview")->setEnabled(false);
	delete m_puzzlePreview;
	m_puzzlePreview = 0;
	m_mainWindow->setCaption(QString());
}

void Palapeli::GamePlay::actionTogglePreview()
{
	if (m_puzzlePreview) {
		m_puzzlePreview->toggleVisible();
		m_mainWindow->actionCollection()->action("toggle_preview")->
			setChecked(Settings::puzzlePreviewVisible());
	}
}

void Palapeli::GamePlay::actionCreate()
{
	QPointer<Palapeli::PuzzleCreatorDialog> creatorDialog(new Palapeli::PuzzleCreatorDialog);
	if (creatorDialog->exec())
	{
		if (!creatorDialog)
			return;
		Palapeli::Puzzle* puzzle = creatorDialog->result();
		if (!puzzle) {
			delete creatorDialog;
			return;
		}
		Palapeli::Collection::instance()->importPuzzle(puzzle);
		playPuzzle(puzzle);
	}
	delete creatorDialog;
}

void Palapeli::GamePlay::actionDelete()
{
	QModelIndexList indexes = m_collectionView->selectedIndexes();
	//ask user for confirmation
	QStringList puzzleNames;
	foreach (const QModelIndex& index, indexes)
		puzzleNames << index.data(Qt::DisplayRole).toString();
	const int result = KMessageBox::warningContinueCancelList(m_mainWindow, i18n("The following puzzles will be deleted. This action cannot be undone."), puzzleNames);
	if (result != KMessageBox::Continue)
		return;
	//do deletion
	Palapeli::Collection* coll = Palapeli::Collection::instance();
	foreach (const QModelIndex& index, indexes)
		coll->deletePuzzle(index);
}

void Palapeli::GamePlay::actionImport()
{
	const QString filter = i18nc("Filter for a file dialog", "*.puzzle|Palapeli puzzles (*.puzzle)");
	const QStringList paths = KFileDialog::getOpenFileNames(KUrl("kfiledialog:///palapeli-import"), filter);
	Palapeli::Collection* coll = Palapeli::Collection::instance();
	foreach (const QString& path, paths)
		coll->importPuzzle(path);
}

void Palapeli::GamePlay::actionExport()
{
	QModelIndexList indexes = m_collectionView->selectedIndexes();
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

void Palapeli::GamePlay::toggleCloseUp()
{
	m_puzzleTable->view()->toggleCloseUp();
}

void Palapeli::GamePlay::configure()
{
	Palapeli::ConfigDialog().exec();
}

//END action handlers

#include "gameplay.moc"

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

#include "mainwindow.h"
#include "puzzletablewidget.h"
#include "../config/configdialog.h"
#include "../creator/puzzlecreator.h"
#include "../engine/scene.h"
#include "../engine/view.h"
#include "../file-io/collection.h"
#include "../file-io/collection-view.h"
#include "../file-io/components.h"
#include "../file-io/puzzle.h"
#include "settings.h"

#include <QtGui/QStackedWidget>
#include <KDE/KAction>
#include <KDE/KActionCollection>
#include <KDE/KCmdLineArgs>
#include <KDE/KFileDialog>
#include <KDE/KLocalizedString>
#include <KDE/KMessageBox>
#include <KDE/KStandardAction>
#include <KDE/KToggleAction>

//TODO: move LoadingWidget into here (stack into m_centralWidget)

Palapeli::MainWindow::MainWindow(KCmdLineArgs* args)
	: m_centralWidget(new QStackedWidget)
	, m_collectionView(new Palapeli::CollectionView)
	, m_puzzleTable(new Palapeli::PuzzleTableWidget)
{
	setupActions();
	//setup GUI
	KXmlGuiWindow::StandardWindowOptions guiOptions = KXmlGuiWindow::Default;
	guiOptions &= ~KXmlGuiWindow::StatusBar; //no statusbar
	setupGUI(QSize(500, 500), guiOptions);
	//setup collection view
	m_collectionView->setModel(Palapeli::Collection::instance());
	connect(m_collectionView, SIGNAL(playRequest(Palapeli::Puzzle*)), SLOT(playPuzzle(Palapeli::Puzzle*)));
	//setup puzzle table
	m_puzzleTable->showStatusBar(Settings::showStatusBar());
	//setup central widget
	m_centralWidget->addWidget(m_collectionView);
	m_centralWidget->addWidget(m_puzzleTable);
	m_centralWidget->setCurrentWidget(m_collectionView);
	setCentralWidget(m_centralWidget);
	//start a puzzle if a puzzle URL has been given
	if (args->count() != 0)
	{
		const QString path = args->arg(0);
		const QString id = Palapeli::Puzzle::fsIdentifier(path);
		playPuzzle(new Palapeli::Puzzle(new Palapeli::ArchiveStorageComponent, path, id));
	}
	args->clear();
}

void Palapeli::MainWindow::setupActions()
{
	//standard stuff
	KStandardAction::preferences(this, SLOT(configure()), actionCollection());
	KAction* statusBarAct = KStandardAction::showStatusbar(m_puzzleTable, SLOT(showStatusBar(bool)), actionCollection());
	statusBarAct->setChecked(Settings::showStatusBar());
	statusBarAct->setText(i18n("Show statusbar of puzzle table"));
	//Back to collection
	KAction* goCollAct = new KAction(KIcon("go-previous"), i18n("Back to &collection"), 0);
	goCollAct->setToolTip(i18n("Go back to the collection to choose another puzzle"));
	goCollAct->setEnabled(false); //because the collection is initially shown
	actionCollection()->addAction("go_collection", goCollAct);
	connect(goCollAct, SIGNAL(triggered()), SLOT(actionGoCollection()));
	//Create new puzzle (FIXME: action should have a custom icon)
	KAction* createAct = new KAction(KIcon("tools-wizard"), i18n("Create &new puzzle..."), 0);
	createAct->setShortcut(KStandardShortcut::openNew());
	createAct->setToolTip(i18n("Create a new puzzle using an image file from your disk"));
	actionCollection()->addAction("file_new", createAct);
	connect(createAct, SIGNAL(triggered()), SLOT(actionCreate()));
	//Delete
	KAction* deleteAct = new KAction(KIcon("archive-remove"), i18n("&Delete"), 0);
	deleteAct->setEnabled(false); //will be enabled when something is selected
	deleteAct->setToolTip(i18n("Delete the selected puzzle from your collection"));
	actionCollection()->addAction("file_delete", deleteAct);
	connect(m_collectionView, SIGNAL(canDeleteChanged(bool)), deleteAct, SLOT(setEnabled(bool)));
	connect(deleteAct, SIGNAL(triggered()), SLOT(actionDelete()));
	//Import from file...
	KAction* importAct = new KAction(KIcon("document-import"), i18n("&Import from file..."), 0);
	importAct->setToolTip(i18n("Import a new puzzle from a file into your collection"));
	actionCollection()->addAction("file_import", importAct);
	connect(importAct, SIGNAL(triggered()), this, SLOT(actionImport()));
	//Export to file...
	KAction* exportAct = new KAction(KIcon("document-export"), i18n("&Export to file..."), 0);
	exportAct->setEnabled(false); //will be enabled when something is selected
	exportAct->setToolTip(i18n("Export the selected puzzle from your collection into a file"));
	actionCollection()->addAction("file_export", exportAct);
	connect(m_collectionView, SIGNAL(canExportChanged(bool)), exportAct, SLOT(setEnabled(bool)));
	connect(exportAct, SIGNAL(triggered()), this, SLOT(actionExport()));
	//Restart puzzle (TODO: placed here only temporarily)
	KAction* restartPuzzleAct = new KAction(KIcon("view-refresh"), i18n("&Restart puzzle..."), 0);
	restartPuzzleAct->setToolTip(i18n("Delete the saved progress"));
	actionCollection()->addAction("game_restart", restartPuzzleAct);
	connect(restartPuzzleAct, SIGNAL(triggered()), m_puzzleTable->view()->scene(), SLOT(restartPuzzle()));
	KAction* toggleCloseUpAct = new KAction(i18nc("As in a movie close-up scene", "Close-up View"), 0);
	toggleCloseUpAct->setShortcut(QKeySequence(Qt::Key_Space));
	actionCollection()->addAction("toggle_closeup", toggleCloseUpAct);
	connect(toggleCloseUpAct, SIGNAL(triggered()), m_puzzleTable->view(), SLOT(toggleCloseUp()));
}

//BEGIN action handlers

void Palapeli::MainWindow::configure()
{
	Palapeli::ConfigDialog().exec();
}

void Palapeli::MainWindow::playPuzzle(Palapeli::Puzzle* puzzle)
{
	if (!puzzle)
		return;
	m_puzzleTable->view()->scene()->loadPuzzle(puzzle);
	m_centralWidget->setCurrentWidget(m_puzzleTable);
	actionCollection()->action("go_collection")->setEnabled(true);
	//load caption from metadata
	puzzle->get(Palapeli::PuzzleComponent::Metadata).waitForFinished();
	const Palapeli::MetadataComponent* cmp = puzzle->component<Palapeli::MetadataComponent>();
	setCaption(cmp ? cmp->metadata.name : QString());
}

void Palapeli::MainWindow::actionGoCollection()
{
	m_centralWidget->setCurrentWidget(m_collectionView);
	actionCollection()->action("go_collection")->setEnabled(false);
	setCaption(QString());
}

void Palapeli::MainWindow::actionCreate()
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

void Palapeli::MainWindow::actionDelete()
{
	QModelIndexList indexes = m_collectionView->selectedIndexes();
	//ask user for confirmation
	QStringList puzzleNames;
	foreach (const QModelIndex& index, indexes)
		puzzleNames << index.data(Qt::DisplayRole).toString();
	const int result = KMessageBox::warningContinueCancelList(this, i18n("The following puzzles will be deleted. This action cannot be undone."), puzzleNames);
	if (result != KMessageBox::Continue)
		return;
	//do deletion
	Palapeli::Collection* coll = Palapeli::Collection::instance();
	foreach (const QModelIndex& index, indexes)
		coll->deletePuzzle(index);
}

void Palapeli::MainWindow::actionImport()
{
	const QString filter = i18nc("Filter for a file dialog", "*.puzzle|Palapeli puzzles (*.puzzle)");
	const QStringList paths = KFileDialog::getOpenFileNames(KUrl("kfiledialog:///palapeli-import"), filter);
	Palapeli::Collection* coll = Palapeli::Collection::instance();
	foreach (const QString& path, paths)
		coll->importPuzzle(path);
}

void Palapeli::MainWindow::actionExport()
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

//END action handlers

#include "mainwindow.moc"

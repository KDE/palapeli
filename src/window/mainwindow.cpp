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

#include "../engine/gameplay.h"

#include "puzzletablewidget.h"
#include "../file-io/collection-view.h"
#include "settings.h"

#include <KDE/KAction>
#include <KDE/KActionCollection>
#include <KDE/KCmdLineArgs>
#include <KDE/KLocalizedString>
#include <KDE/KStandardAction>
#include <KDE/KToggleAction>

Palapeli::MainWindow::MainWindow(KCmdLineArgs* args)
	: m_game(new Palapeli::GamePlay(this))
{
	setupActions();
	//setup GUI
	KXmlGuiWindow::StandardWindowOptions guiOptions = KXmlGuiWindow::Default;
	guiOptions &= ~KXmlGuiWindow::StatusBar; //no statusbar
	setupGUI(QSize(500, 500), guiOptions);
	m_game->init();
	//start a puzzle if a puzzle URL has been given
	if (args->count() != 0)
	{
		m_game->playPuzzleFile(args->arg(0));
	}
	args->clear();
}

void Palapeli::MainWindow::setupActions()
{
	// Standard stuff.
	KStandardAction::preferences(m_game, SLOT(configure()),
						actionCollection());
	KAction* statusBarAct = KStandardAction::showStatusbar
	(m_game->puzzleTable(), SLOT(showStatusBar(bool)), actionCollection());
	statusBarAct->setChecked(Settings::showStatusBar());
	statusBarAct->setText(i18n("Show statusbar of puzzle table"));

	// Back to collection.
	KAction* goCollAct = new KAction(KIcon("go-previous"), i18n("Back to &collection"), 0);
	goCollAct->setToolTip(i18n("Go back to the collection to choose another puzzle"));
	goCollAct->setEnabled(false); //because the collection is initially shown
	actionCollection()->addAction("view_collection", goCollAct);
	connect(goCollAct, SIGNAL(triggered()), m_game, SLOT(actionGoCollection()));

	// Create new puzzle (FIXME: action should have a custom icon).
	KAction* createAct = new KAction(KIcon("tools-wizard"), i18n("Create &new puzzle..."), 0);
	createAct->setShortcut(KStandardShortcut::openNew());
	createAct->setToolTip(i18n("Create a new puzzle using an image file from your disk"));
	actionCollection()->addAction("game_new", createAct);
	connect(createAct, SIGNAL(triggered()), m_game, SLOT(actionCreate()));

	// Delete a puzzle.
	KAction* deleteAct = new KAction(KIcon("archive-remove"), i18n("&Delete puzzle"), 0);
	deleteAct->setEnabled(false); //will be enabled when something is selected
	deleteAct->setToolTip(i18n("Delete the selected puzzle from your collection"));
	actionCollection()->addAction("game_delete", deleteAct);
	connect(m_game->collectionView(), SIGNAL(canDeleteChanged(bool)), deleteAct, SLOT(setEnabled(bool)));
	connect(deleteAct, SIGNAL(triggered()), m_game, SLOT(actionDelete()));

	// Import from file...
	KAction* importAct = new KAction(KIcon("document-import"), i18n("&Import from file..."), 0);
	importAct->setToolTip(i18n("Import a new puzzle from a file into your collection"));
	actionCollection()->addAction("game_import", importAct);
	connect(importAct, SIGNAL(triggered()), m_game, SLOT(actionImport()));

	// Export to file...
	KAction* exportAct = new KAction(KIcon("document-export"), i18n("&Export to file..."), 0);
	exportAct->setEnabled(false); //will be enabled when something is selected
	exportAct->setToolTip(i18n("Export the selected puzzle from your collection into a file"));
	actionCollection()->addAction("game_export", exportAct);
	connect(m_game->collectionView(), SIGNAL(canExportChanged(bool)), exportAct, SLOT(setEnabled(bool)));
	connect(exportAct, SIGNAL(triggered()), m_game, SLOT(actionExport()));

	//Reshuffle and restart puzzle
	KAction* restartPuzzleAct = new KAction(KIcon("view-refresh"), i18n("&Restart puzzle..."), 0);
	restartPuzzleAct->setToolTip(i18n("Delete the saved progress and reshuffle the pieces"));
	restartPuzzleAct->setEnabled(false); //no puzzle in progress initially
	actionCollection()->addAction("game_restart", restartPuzzleAct);
	connect(restartPuzzleAct, SIGNAL(triggered()), m_game, SLOT(restartPuzzle()));
	// Create piece-holder.
	KAction* createHolderAct = new KAction(i18n("&Create piece holder..."), 0);
	createHolderAct->setToolTip(i18n("Create a temporary holder for sorting pieces"));
	createHolderAct->setShortcut(QKeySequence(Qt::Key_C));
	actionCollection()->addAction("move_create_holder", createHolderAct);
	connect(createHolderAct, SIGNAL(triggered()), m_game, SLOT(createHolder()));

	// Delete piece-holder.
	KAction* deleteHolderAct = new KAction(i18n("&Delete piece holder"), 0);
	deleteHolderAct->setToolTip(i18n("Delete a selected temporary holder when it is empty"));
	deleteHolderAct->setShortcut(QKeySequence(Qt::Key_D));
	actionCollection()->addAction("move_delete_holder", deleteHolderAct);
	connect(deleteHolderAct, SIGNAL(triggered()), m_game, SLOT(deleteHolder()));

	// Select all pieces in a piece-holder.
	KAction* selectAllAct = new KAction(i18n("&Select all in holder"), 0);
	selectAllAct->setToolTip(i18n("Select all pieces in a selected piece holder"));
	selectAllAct->setShortcut(QKeySequence(Qt::Key_A));
	actionCollection()->addAction("move_select_all", selectAllAct);
	connect(selectAllAct, SIGNAL(triggered()), m_game, SLOT(selectAll()));

	// Rearrange a selected piece-holder or selected pieces in any view.
	KAction* rearrangeAct = new KAction(i18n("&Rearrange pieces"), 0);
	rearrangeAct->setToolTip(i18n("Rearrange all pieces in a selected piece holder or selected pieces in any window"));
	rearrangeAct->setShortcut(QKeySequence(Qt::Key_R));
	actionCollection()->addAction("move_rearrange", rearrangeAct);
	connect(rearrangeAct, SIGNAL(triggered()), m_game, SLOT(rearrangePieces()));

	// Toggle puzzle-preview.
	bool  isVisible = Settings::puzzlePreviewVisible();
	const QString text = i18nc("Preview is a noun here", "&Preview");
	KToggleAction* togglePreviewAct = new KToggleAction(KIcon("view-preview"), text, 0);
	togglePreviewAct->setIconText(i18nc("Preview is a noun here", "Preview"));
	togglePreviewAct->setToolTip(i18n("Shows or hides the image of the completed puzzle"));
	actionCollection()->addAction("view_preview", togglePreviewAct);
	togglePreviewAct->setEnabled(false);
	togglePreviewAct->setChecked(isVisible);
	connect(togglePreviewAct, SIGNAL(triggered()), m_game, SLOT(actionTogglePreview()));

	/* REMOVED: Now triggered by mouse button, default Middle-Click.
	// Toggle close-up view.
	KToggleAction* toggleCloseUpAct = new KToggleAction(i18nc("As in a movie close-up scene", "Close-up View"), 0);
	toggleCloseUpAct->setShortcut(QKeySequence(Qt::Key_Space));
	actionCollection()->addAction("view_closeup", toggleCloseUpAct);
	connect(toggleCloseUpAct, SIGNAL(triggered()), m_game, SLOT(toggleCloseUp()));
	*/

	// View zoom in.
	KStandardAction::zoomIn(m_game, SLOT(actionZoomIn()),
						actionCollection());

	// View zoom out.
	KStandardAction::zoomOut(m_game, SLOT(actionZoomOut()),
						actionCollection());
}

#include "mainwindow.moc"

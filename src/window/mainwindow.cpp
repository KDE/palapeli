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
#include <QAction>
#include <KActionCollection>
#include <KLocalizedString>
#include <KStandardAction>
#include <KStandardShortcut>
#include <KToggleAction>
#include <KMessageBox>

Palapeli::MainWindow::MainWindow(const QString &path)
	: m_game(new Palapeli::GamePlay(this))
{
	setupActions();
	//setup GUI
	KXmlGuiWindow::StandardWindowOptions guiOptions = KXmlGuiWindow::Default;
	guiOptions &= ~KXmlGuiWindow::StatusBar; //no statusbar
	setupGUI(QSize(500, 500), guiOptions);
	m_game->init();
	//start a puzzle if a puzzle URL has been given
	if (!path.isEmpty())
		m_game->playPuzzleFile(path);
}

bool Palapeli::MainWindow::queryClose()
{
	// Terminate cleanly if the user Quits when playing a puzzle.
	m_game->shutdown();
	return true;
}

void Palapeli::MainWindow::setupActions()
{
	// Standard stuff.
	KStandardAction::preferences(m_game, SLOT(configure()),
						actionCollection());
	QAction * statusBarAct = KStandardAction::showStatusbar
	(m_game->puzzleTable(), SLOT(showStatusBar(bool)), actionCollection());
	statusBarAct->setChecked(Settings::showStatusBar());
	statusBarAct->setText(i18n("Show statusbar of puzzle table"));

	// Back to collection.
	QAction * goCollAct = new QAction(QIcon::fromTheme("go-previous"), i18n("Back to &collection"), this);
	goCollAct->setToolTip(i18n("Go back to the collection to choose another puzzle"));
	goCollAct->setEnabled(false); //because the collection is initially shown
	actionCollection()->addAction("view_collection", goCollAct);
	connect(goCollAct, SIGNAL(triggered()), m_game, SLOT(actionGoCollection()));

	// Create new puzzle (FIXME: action should have a custom icon).
	QAction * createAct = new QAction(QIcon::fromTheme("tools-wizard"), i18n("Create &new puzzle..."), this);
	createAct->setToolTip(i18n("Create a new puzzle using an image file from your disk"));
	actionCollection()->setDefaultShortcuts(createAct, KStandardShortcut::openNew());
	actionCollection()->addAction("game_new", createAct);
	connect(createAct, SIGNAL(triggered()), m_game, SLOT(actionCreate()));

	// Delete a puzzle.
	QAction * deleteAct = new QAction(QIcon::fromTheme("archive-remove"), i18n("&Delete puzzle"), this);
	deleteAct->setEnabled(false); //will be enabled when something is selected
	deleteAct->setToolTip(i18n("Delete the selected puzzle from your collection"));
	actionCollection()->addAction("game_delete", deleteAct);
	connect(m_game->collectionView(), SIGNAL(canDeleteChanged(bool)), deleteAct, SLOT(setEnabled(bool)));
	connect(deleteAct, SIGNAL(triggered()), m_game, SLOT(actionDelete()));

	// Import from file...
	QAction * importAct = new QAction(QIcon::fromTheme("document-import"), i18n("&Import from file..."), this);
	importAct->setToolTip(i18n("Import a new puzzle from a file into your collection"));
	actionCollection()->addAction("game_import", importAct);
	connect(importAct, SIGNAL(triggered()), m_game, SLOT(actionImport()));

	// Export to file...
	QAction * exportAct = new QAction(QIcon::fromTheme("document-export"), i18n("&Export to file..."), this);
	exportAct->setEnabled(false); //will be enabled when something is selected
	exportAct->setToolTip(i18n("Export the selected puzzle from your collection into a file"));
	actionCollection()->addAction("game_export", exportAct);
	connect(m_game->collectionView(), SIGNAL(canExportChanged(bool)), exportAct, SLOT(setEnabled(bool)));
	connect(exportAct, SIGNAL(triggered()), m_game, SLOT(actionExport()));

	//Reshuffle and restart puzzle
	QAction * restartPuzzleAct = new QAction(QIcon::fromTheme("view-refresh"), i18n("&Restart puzzle..."), this);
	restartPuzzleAct->setToolTip(i18n("Delete the saved progress and reshuffle the pieces"));
	restartPuzzleAct->setEnabled(false); //no puzzle in progress initially
	actionCollection()->addAction("game_restart", restartPuzzleAct);
	connect(restartPuzzleAct, SIGNAL(triggered()), m_game, SLOT(restartPuzzle()));
	// Quit.
	KStandardAction::quit (this, SLOT (close()), actionCollection());
	// Create piece-holder.
	QAction * createHolderAct = new QAction(i18n("&Create piece holder..."), this);
	createHolderAct->setToolTip(i18n("Create a temporary holder for sorting pieces"));
	actionCollection()->setDefaultShortcut(createHolderAct, QKeySequence(Qt::Key_C));
	actionCollection()->addAction("move_create_holder", createHolderAct);
	connect(createHolderAct, SIGNAL(triggered()), m_game, SLOT(createHolder()));

	// Delete piece-holder.
	QAction * deleteHolderAct = new QAction(i18n("&Delete piece holder"), this);
	deleteHolderAct->setToolTip(i18n("Delete a selected temporary holder when it is empty"));
	actionCollection()->setDefaultShortcut(deleteHolderAct, QKeySequence(Qt::Key_D));
	actionCollection()->addAction("move_delete_holder", deleteHolderAct);
	connect(deleteHolderAct, SIGNAL(triggered()), m_game, SLOT(deleteHolder()));

	// Select all pieces in a piece-holder.
	QAction * selectAllAct = new QAction(i18n("&Select all in holder"), this);
	selectAllAct->setToolTip(i18n("Select all pieces in a selected piece holder"));
	actionCollection()->setDefaultShortcut(selectAllAct, QKeySequence(Qt::Key_A));
	actionCollection()->addAction("move_select_all", selectAllAct);
	connect(selectAllAct, SIGNAL(triggered()), m_game, SLOT(selectAll()));

	// Rearrange a selected piece-holder or selected pieces in any view.
	QAction * rearrangeAct = new QAction(i18n("&Rearrange pieces"), this);
	rearrangeAct->setToolTip(i18n("Rearrange all pieces in a selected piece holder or selected pieces in any window"));
	actionCollection()->setDefaultShortcut(rearrangeAct, QKeySequence(Qt::Key_R));
	actionCollection()->addAction("move_rearrange", rearrangeAct);
	connect(rearrangeAct, SIGNAL(triggered()), m_game, SLOT(rearrangePieces()));

	// Toggle puzzle-preview.
	bool  isVisible = Settings::puzzlePreviewVisible();
	const QString text = i18nc("Preview is a noun here", "&Preview");
	KToggleAction* togglePreviewAct = new KToggleAction(QIcon::fromTheme("view-preview"), text, 0);
	togglePreviewAct->setIconText(i18nc("Preview is a noun here", "Preview"));
	togglePreviewAct->setToolTip(i18n("Show or hide the image of the completed puzzle"));
	actionCollection()->addAction("view_preview", togglePreviewAct);
	togglePreviewAct->setEnabled(false);
	togglePreviewAct->setChecked(false);
	connect(togglePreviewAct, SIGNAL(triggered()), m_game, SLOT(actionTogglePreview()));

	// View zoom in.
	KStandardAction::zoomIn(m_game, SLOT(actionZoomIn()),
						actionCollection());

	// View zoom out.
	KStandardAction::zoomOut(m_game, SLOT(actionZoomOut()),
						actionCollection());
	// Settings: enable messages that the user marked "Do not show again".
	QAction * enableMessagesAct = new QAction(i18n("Enable all messages"), this);
	actionCollection()->addAction("enable_messages", enableMessagesAct);
	connect(enableMessagesAct, SIGNAL(triggered()), SLOT(enableMessages()));
}

void Palapeli::MainWindow::enableMessages()
{
	// Enable all messages that the user has marked "Do not show again".
	int result = KMessageBox::questionYesNo(this,
					i18n("Enable all messages"));
	if (result == KMessageBox::Yes) {
		qDebug() << "ENABLE ALL MESSAGES";
		KMessageBox::enableAllMessages();
		KSharedConfig::openConfig()->sync();	// Save the changes to disk.
	}
}



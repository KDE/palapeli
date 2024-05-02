/*
    SPDX-FileCopyrightText: 2009-2011 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mainwindow.h"

#include "../engine/gameplay.h"
#include "palapeli_debug.h"

#include "puzzletablewidget.h"
#include "../file-io/collection-view.h"
#include "settings.h"
#include <QAction>

#include <KActionCollection>
#include <KLocalizedString>
#include <KGameStandardAction>
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
	KStandardAction::preferences(m_game, &GamePlay::configure,
						actionCollection());
	QAction * statusBarAct = KStandardAction::showStatusbar
	(m_game->puzzleTable(), &PuzzleTableWidget::showStatusBar, actionCollection());
	statusBarAct->setChecked(Settings::showStatusBar());
	statusBarAct->setText(i18n("Show Statusbar of Puzzle Table"));

	// Back to collection.
	auto *goCollAct = new QAction(QIcon::fromTheme(QStringLiteral("go-previous")),
				      i18nc("@action", "Back to &Collection"), this);
	goCollAct->setToolTip(i18nc("@info:tooltip", "Go back to the collection to choose another puzzle"));
	goCollAct->setEnabled(false); //because the collection is initially shown
	actionCollection()->addAction(QStringLiteral("view_collection"), goCollAct);
	connect(goCollAct, &QAction::triggered, m_game, &GamePlay::actionGoCollection);

	// Create new puzzle (FIXME: action should have a custom icon).
	auto *createAct = new QAction(QIcon::fromTheme(QStringLiteral("tools-wizard")),
				      i18nc("@action", "Create &New Puzzle…"), this);
	createAct->setToolTip(i18nc("@info:tooltip", "Create a new puzzle using an image file from your disk"));
	KActionCollection::setDefaultShortcuts(createAct, KStandardShortcut::openNew());
	actionCollection()->addAction(QStringLiteral("game_new"), createAct);
	connect(createAct, &QAction::triggered, m_game, &GamePlay::actionCreate);

	// Delete a puzzle.
	auto *deleteAct = new QAction(QIcon::fromTheme(QStringLiteral("edit-delete")),
				      i18nc("@action", "&Delete Puzzle"), this);
	deleteAct->setEnabled(false); //will be enabled when something is selected
	deleteAct->setToolTip(i18nc("@info:tooltip", "Delete the selected puzzle from your collection"));
	actionCollection()->addAction(QStringLiteral("game_delete"), deleteAct);
	connect(m_game->collectionView(), &CollectionView::canDeleteChanged, deleteAct, &QAction::setEnabled);
	connect(deleteAct, &QAction::triggered, m_game, &GamePlay::actionDelete);

	// Import from file...
	auto *importAct = new QAction(QIcon::fromTheme(QStringLiteral("document-import")),
				      i18nc("@action", "&Import from File…"), this);
	importAct->setToolTip(i18nc("@info:tooltip", "Import a new puzzle from a file into your collection"));
	actionCollection()->addAction(QStringLiteral("game_import"), importAct);
	connect(importAct, &QAction::triggered, m_game, &GamePlay::actionImport);

	// Export to file...
	auto *exportAct = new QAction(QIcon::fromTheme(QStringLiteral("document-export")),
				      i18nc("@action", "&Export to File…"), this);
	exportAct->setEnabled(false); //will be enabled when something is selected
	exportAct->setToolTip(i18nc("@info:tooltip", "Export the selected puzzle from your collection into a file"));
	actionCollection()->addAction(QStringLiteral("game_export"), exportAct);
	connect(m_game->collectionView(), &CollectionView::canExportChanged, exportAct, &QAction::setEnabled);
	connect(exportAct, &QAction::triggered, m_game, &GamePlay::actionExport);

	//Reshuffle and restart puzzle
	auto *restartPuzzleAct = new QAction(QIcon::fromTheme(QStringLiteral("view-refresh")),
					     i18nc("@action", "&Restart Puzzle…"), this);
	restartPuzzleAct->setToolTip(i18nc("@info:tooltip", "Delete the saved progress and reshuffle the pieces"));
	restartPuzzleAct->setEnabled(false); //no puzzle in progress initially
	actionCollection()->addAction(QStringLiteral("game_restart"), restartPuzzleAct);
	connect(restartPuzzleAct, &QAction::triggered, m_game, &GamePlay::restartPuzzle);
	// Quit.
	KGameStandardAction::quit (this, &QWidget::close, actionCollection());
	// Create piece-holder.
	auto *createHolderAct = new QAction(i18nc("@action", "&Create Piece Holder…"), this);
	createHolderAct->setToolTip(i18nc("@info:tooltip", "Create a temporary holder for sorting pieces"));
	KActionCollection::setDefaultShortcut(createHolderAct, QKeySequence(Qt::Key_C));
	actionCollection()->addAction(QStringLiteral("move_create_holder"), createHolderAct);
        connect(createHolderAct, &QAction::triggered, m_game,
                qOverload<>(&GamePlay::createHolder));

        // Delete piece-holder.
	auto *deleteHolderAct = new QAction(QIcon::fromTheme(QStringLiteral("edit-delete")),
					    i18nc("@action", "&Delete Piece Holder"), this);
	deleteHolderAct->setToolTip(i18nc("@info:tooltip", "Delete a selected temporary holder when it is empty"));
	KActionCollection::setDefaultShortcut(deleteHolderAct, QKeySequence(Qt::Key_D));
	actionCollection()->addAction(QStringLiteral("move_delete_holder"), deleteHolderAct);
	connect(deleteHolderAct, &QAction::triggered, m_game, &GamePlay::deleteHolder);

	// Select all pieces in a piece-holder.
	auto *selectAllAct = new QAction(QIcon::fromTheme(QStringLiteral("edit-select-all")),
					 i18nc("@action", "&Select All in Holder"), this);
	selectAllAct->setToolTip(i18nc("@info:tooltip", "Select all pieces in a selected piece holder"));
	KActionCollection::setDefaultShortcut(selectAllAct, QKeySequence(Qt::Key_A));
	actionCollection()->addAction(QStringLiteral("move_select_all"), selectAllAct);
	connect(selectAllAct, &QAction::triggered, m_game, &GamePlay::selectAll);

	// Rearrange a selected piece-holder or selected pieces in any view.
	auto *rearrangeAct = new QAction(i18nc("@action", "&Rearrange Pieces"), this);
	rearrangeAct->setToolTip(i18nc("@info:tooltip", "Rearrange all pieces in a selected piece holder or selected pieces in any window"));
	KActionCollection::setDefaultShortcut(rearrangeAct, QKeySequence(Qt::Key_R));
	actionCollection()->addAction(QStringLiteral("move_rearrange"), rearrangeAct);
	connect(rearrangeAct, &QAction::triggered, m_game, &GamePlay::rearrangePieces);

	// Toggle puzzle-preview.
	bool  isVisible = Settings::puzzlePreviewVisible();
	auto *togglePreviewAct = new KToggleAction(QIcon::fromTheme(QStringLiteral("view-preview")),
						   i18nc("@option:check", "Show Preview"), this);
	togglePreviewAct->setIconText(i18nc("@option:check Preview is a noun here", "Preview"));
	togglePreviewAct->setToolTip(i18nc("@info:tooltip", "Show or hide the image of the completed puzzle"));
	actionCollection()->addAction(QStringLiteral("view_preview"), togglePreviewAct);
	togglePreviewAct->setEnabled(false);
	togglePreviewAct->setChecked(isVisible);
	connect(togglePreviewAct, &QAction::triggered, m_game, &GamePlay::actionTogglePreview);

	// View zoom in.
	KStandardAction::zoomIn(m_game, &GamePlay::actionZoomIn,
						actionCollection());

	// View zoom out.
	KStandardAction::zoomOut(m_game, &GamePlay::actionZoomOut,
						actionCollection());
	// Settings: enable messages that the user marked "Do not show again".
	auto *enableMessagesAct = new QAction(i18nc("@action", "Enable All Messages"), this);
	actionCollection()->addAction(QStringLiteral("enable_messages"), enableMessagesAct);
	connect(enableMessagesAct, &QAction::triggered, this, &MainWindow::enableMessages);
}

void Palapeli::MainWindow::enableMessages()
{
	// Enable all messages that the user has marked "Do not show again".
	int result = KMessageBox::questionTwoActions(this,
						i18n("Enable all messages again?"), {},
						KGuiItem(i18nc("@action:button", "Enable"), QStringLiteral("dialog-ok")),
						KStandardGuiItem::cancel());
	if (result == KMessageBox::PrimaryAction) {
		qCDebug(PALAPELI_LOG) << "ENABLE ALL MESSAGES";
		KMessageBox::enableAllMessages();
		KSharedConfig::openConfig()->sync();	// Save the changes to disk.
	}
}

#include "moc_mainwindow.cpp"

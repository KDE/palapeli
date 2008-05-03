/***************************************************************************
 *   Copyright (C) 2008 Felix Lemke <lemke.felix@ages-skripte.org>
 *   Copyright (C) 2008 Stefan Majewsky <majewsky@gmx.net>
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
#include "loadaction.h"
#include "manager.h"
#include "minimap.h"
#include "preview.h"
#include "saveaction.h"
#include "savegameview.h"
#include "ui_dialognew.h"
#include "view.h"

#include <QDockWidget>
#include <QTimer>
#include <KAction>
#include <KActionCollection>
#include <KApplication>
#include <KDialog>
#include <KLocalizedString>
#include <KDE/KStandardGameAction>
#include <KStatusBar>

//TODO: Request a proper icon for the savegame management action.
//TODO: Eventually convert SavegameView from a dialog into a dock widget.
//TODO: Add actions to toggle dock widgets. (Is there a standard way for this?)

Palapeli::MainWindow::MainWindow(Palapeli::Manager* manager, QWidget* parent)
	: KXmlGuiWindow(parent)
	, m_manager(manager)
	, m_loadAct(new Palapeli::LoadAction(m_manager, this))
	, m_saveAct(new Palapeli::SaveAction(m_manager, this))
	, m_manageSavegameAct(new KAction(KIcon("document-save-as"), i18n("Manage saved games"), this))
	, m_dockMinimap(new QDockWidget(i18n("Overview"), this))
	, m_dockPreview(new QDockWidget(i18n("Image preview"), this))
	, m_newDialog(new KDialog(this))
	, m_newUi(new Ui::NewPuzzleDialog)
	, m_savegameDialog(new KDialog(this))
	, m_savegameView(0)
{
	//Game actions
	KStandardGameAction::gameNew(m_newDialog, SLOT(show()), actionCollection());
	KStandardGameAction::quit(kapp, SLOT(quit()), actionCollection());
	//KStandardGameAction::load(this, SLOT(loadGame()), actionCollection());
	//KStandardGameAction::save(this, SLOT(saveGame()), actionCollection());
	actionCollection()->addAction("game_load", m_loadAct);
	actionCollection()->addAction("game_save", m_saveAct);
	actionCollection()->addAction("palapeli_manage_savegames", m_manageSavegameAct);
	//GUI settings
	setAutoSaveSettings();
	setCentralWidget(m_manager->view());
	//minimap
	addDockWidget(Qt::RightDockWidgetArea, m_dockMinimap);
	m_dockMinimap->setObjectName("DockMap");
	m_dockMinimap->setWidget(m_manager->minimap());
	//preview
	addDockWidget(Qt::RightDockWidgetArea, m_dockPreview);
	m_dockPreview->setObjectName("DockPreview");
	m_dockPreview->setWidget(m_manager->preview());
	//late GUI settings
	setupGUI(QSize(400, 400));
	setCaption(i18nc("The application's name", "Palapeli"));
	statusBar()->hide();
	//initialise dialogs after entering the event loop (to speed up startup)
	QTimer::singleShot(0, this, SLOT(setupDialogs()));
}

Palapeli::MainWindow::~MainWindow()
{
	delete m_loadAct;
	delete m_saveAct;
	delete m_dockMinimap;
	delete m_dockPreview;
	delete m_newDialog;
	delete m_newUi;
	delete m_savegameDialog; //also deletes its content widget m_savegameView
}

void Palapeli::MainWindow::setupDialogs()
{
	//setup "New game" UI
	const int minPieceCount = 0;
	const int defaultPieceCount = 8;
	const int maxPieceCount = 100;
	m_newUi->setupUi(m_newDialog->mainWidget());
	m_newUi->spinHorizontalPieces->setMinimum(minPieceCount);
	m_newUi->spinHorizontalPieces->setMaximum(maxPieceCount);
	m_newUi->spinHorizontalPieces->setValue(defaultPieceCount);
	m_newUi->spinVerticalPieces->setMinimum(minPieceCount);
	m_newUi->spinVerticalPieces->setMaximum(maxPieceCount);
	m_newUi->spinVerticalPieces->setValue(defaultPieceCount);
	//setup "New game" dialog
	m_newDialog->setWindowIcon(KIcon("document-new"));
	m_newDialog->setCaption(i18n("New puzzle"));
	m_newDialog->setButtons(KDialog::Ok | KDialog::Cancel);
	m_newDialog->mainWidget()->layout()->setMargin(0);
	connect(m_newDialog, SIGNAL(okClicked()), this, SLOT(startGame()));
	//setup "Manage savegames" UI
	m_savegameView = new SavegameView(m_manager);
	//setup "Manage savegames" dialog
	m_savegameDialog->setWindowIcon(KIcon("document-save-as"));
	m_savegameDialog->setCaption(i18n("Manage savegames"));
	m_savegameDialog->setButtons(KDialog::Close);
	m_savegameDialog->setMainWidget(m_savegameView);
	connect(m_manageSavegameAct, SIGNAL(triggered()), m_savegameDialog, SLOT(show()));
}

void Palapeli::MainWindow::startGame()
{
	m_manager->createGame(m_newUi->urlImage->url(), m_newUi->spinHorizontalPieces->value(), m_newUi->spinVerticalPieces->value());
}

void Palapeli::MainWindow::loadGame()
{
	m_manager->loadGame("My Savegame");
}

void Palapeli::MainWindow::saveGame()
{
	m_manager->saveGame("My Savegame");
}

#include "mainwindow.moc"

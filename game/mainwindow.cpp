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
#include "mainwindow_p.h"
#include "manager.h"
#include "minimap.h"
#include "preview.h"
#include "savegamemodel.h"
#include "settings.h"
#include "view.h"

#include <QTimer>
#include <KActionCollection>
#include <KLocalizedString>
#include <KDE/KStandardGameAction>
#include <KStatusBar>

Palapeli::MainWindowPrivate::MainWindowPrivate(Palapeli::MainWindow* parent)
	: m_parent(parent)
	, m_loadAct(new Palapeli::ListMenu(KIcon("document-open"), i18n("Load"), parent))
	, m_saveAct(new Palapeli::SaveAction(parent))
	, m_showSavegamesAct(new KAction(KIcon("document-save-as"), i18n("Manage saved games"), parent))
	, m_toggleMinimapAct(new KAction(i18n("Show minimap"), parent))
	, m_togglePreviewAct(new KAction(i18n("Show preview"), parent))
	, m_dockMinimap(new QDockWidget(i18n("Overview"), parent))
	, m_dockPreview(new QDockWidget(i18n("Image preview"), parent))
	, m_savegameView(new Palapeli::SavegameView)
	, m_dockSavegames(new QDockWidget(i18n("Saved games"), parent))
	, m_newDialog(0) //cannot be created until Manager has loaded the pattern plugins
	, m_settingsDialog(new KDialog(parent))
	, m_settingsUi(new Ui::SettingsWidget)
	, m_universalProgress(new Palapeli::TextProgressBar)
{
}

Palapeli::MainWindowPrivate::~MainWindowPrivate()
{
	//actions
	delete m_loadAct;
	delete m_saveAct;
	delete m_showSavegamesAct;
	delete m_toggleMinimapAct;
	delete m_togglePreviewAct;
	//docker widgets
	delete m_dockMinimap;
	delete m_dockPreview;
	delete m_dockSavegames;
	//docker contents
	delete m_savegameView;
	//dialogs
	delete m_newDialog;
	delete m_settingsDialog;
	delete m_settingsUi;
	//status bar
	delete m_universalProgress;
}

void Palapeli::MainWindowPrivate::setupActions()
{
	//Game actions
	KStandardGameAction::gameNew(0, "", m_parent->actionCollection()); //no slot given because m_newDialog is not available yet
	m_parent->actionCollection()->addAction("game_load", m_loadAct);
	m_loadAct->setDelayed(false);
	m_loadAct->setStickyMenu(true);
	m_loadAct->setDisabledWhenEmpty(true);
	m_loadAct->setModel(ppMgr()->savegameModel());
	connect(m_loadAct, SIGNAL(clicked(const QString&)), ppMgr(), SLOT(loadGame(const QString&)));
	m_parent->actionCollection()->addAction("game_save", m_saveAct);
	m_parent->actionCollection()->addAction("palapeli_manage_savegames", m_showSavegamesAct);
	connect(m_showSavegamesAct, SIGNAL(triggered()), m_dockSavegames, SLOT(show()));
	//View actions
	m_parent->actionCollection()->addAction("view_toggle_minimap", m_toggleMinimapAct);
	m_toggleMinimapAct->setCheckable(true);
	m_toggleMinimapAct->setChecked(false);
	connect(m_dockMinimap, SIGNAL(visibilityChanged(bool)), m_toggleMinimapAct, SLOT(setChecked(bool)));
	connect(m_toggleMinimapAct, SIGNAL(triggered(bool)), m_dockMinimap, SLOT(setVisible(bool)));
	m_parent->actionCollection()->addAction("view_toggle_preview", m_togglePreviewAct);
	m_togglePreviewAct->setCheckable(true);
	m_togglePreviewAct->setChecked(false);
	connect(m_dockPreview, SIGNAL(visibilityChanged(bool)), m_togglePreviewAct, SLOT(setChecked(bool)));
	connect(m_togglePreviewAct, SIGNAL(triggered(bool)), m_dockPreview, SLOT(setVisible(bool)));
	//Settings actions
        KStandardAction::preferences(m_settingsDialog, SLOT(show()), m_parent->actionCollection());
}

void Palapeli::MainWindowPrivate::setupDockers()
{
	//minimap
	m_parent->addDockWidget(Qt::LeftDockWidgetArea, m_dockMinimap);
	m_dockMinimap->setObjectName("DockMap");
	m_dockMinimap->setWidget(ppMgr()->minimap());
	m_dockMinimap->resize(1, 1); //lets the dock widget adapt to the content's minimum size (note that this minimum size will be overwritten by user configuration)
        m_dockMinimap->setVisible(false); //hidden by default
	//preview
	m_parent->addDockWidget(Qt::LeftDockWidgetArea, m_dockPreview);
	m_dockPreview->setObjectName("DockPreview");
	m_dockPreview->setWidget(ppMgr()->preview());
	m_dockPreview->resize(1, 1);
	m_dockPreview->setVisible(false); //hidden by default
	//saved games view
	m_parent->addDockWidget(Qt::RightDockWidgetArea, m_dockSavegames);
	m_dockSavegames->setObjectName("DockSavegames");
	m_dockSavegames->setWidget(m_savegameView);
	m_dockSavegames->resize(1, 1);
	m_dockSavegames->setVisible(false); //hidden by default
}

void Palapeli::MainWindowPrivate::setupDialogs()
{
	//setup "New game" dialog - it is now safe to do that because Manager has loaded its pattern plugins
	m_newDialog = new Palapeli::NewPuzzleDialog;
	connect(m_parent->actionCollection()->action(KStandardGameAction::name(KStandardGameAction::New)), SIGNAL(triggered()), m_newDialog, SLOT(show()));
	connect(m_newDialog, SIGNAL(startGame(const KUrl&, int)), ppMgr(), SLOT(createGame(const KUrl&, int)));
	//setup Settings UI
	m_settingsUi->setupUi(m_settingsDialog->mainWidget());
	m_settingsUi->checkAntialiasing->setCheckState(Settings::antialiasing() ? Qt::Checked : Qt::Unchecked);
	connect(m_settingsUi->checkAntialiasing, SIGNAL(stateChanged(int)), this, SLOT(configurationChanged()));
	m_settingsUi->checkHardwareAccel->setCheckState(Settings::hardwareAccel() ? Qt::Checked : Qt::Unchecked);
	connect(m_settingsUi->checkHardwareAccel, SIGNAL(stateChanged(int)), this, SLOT(configurationChanged()));
#ifndef PALAPELI_WITH_OPENGL
	m_settingsUi->checkHardwareAccel->setVisible(false);
#endif
	//setup Settings dialog
	m_settingsDialog->setWindowIcon(KIcon("configure"));
	m_settingsDialog->setCaption(i18n("Configure Palapeli"));
	m_settingsDialog->setButtons(KDialog::Ok | KDialog::Apply | KDialog::Cancel);
	m_settingsDialog->mainWidget()->layout()->setMargin(0);
	m_settingsDialog->enableButtonApply(false); //not until something has changed
	connect(m_settingsDialog, SIGNAL(okClicked()), this, SLOT(configurationFinished()));
	connect(m_settingsDialog, SIGNAL(applyClicked()), this, SLOT(configurationFinished()));
}

void Palapeli::MainWindowPrivate::configurationChanged() //because of user-invoked changes in the dialog
{
	m_settingsDialog->enableButtonApply(true);
}

void Palapeli::MainWindowPrivate::configurationFinished()
{
	//apply settings if they changed
	ppMgr()->view()->setAntialiasing(m_settingsUi->checkAntialiasing->checkState() == Qt::Checked);
	ppMgr()->view()->setHardwareAccelerated(m_settingsUi->checkHardwareAccel->checkState() == Qt::Checked);
	//mark settings as saved in the dialog
	m_settingsDialog->enableButtonApply(false);
}

Palapeli::MainWindow::MainWindow(QWidget* parent)
	: KXmlGuiWindow(parent)
	, p(new Palapeli::MainWindowPrivate(this))
{
	//initialize actions
	p->setupActions();
	//early GUI settings
	setAutoSaveSettings();
	setCentralWidget(ppMgr()->view());
	//initiailize dockers
	p->setupDockers();
	//late GUI settings
	setupGUI(QSize(600, 500));
	setCaption(i18nc("The application's name", "Palapeli"));
	//status bar
	statusBar()->show();
	statusBar()->addPermanentWidget(p->m_universalProgress, 1);
	p->m_universalProgress->setText(i18n("Welcome to Palapeli."));
	//initialise dialogs after entering the event loop (to speed up startup)
	QTimer::singleShot(0, p, SLOT(setupDialogs()));
}

Palapeli::MainWindow::~MainWindow()
{
	delete p;
}

void Palapeli::MainWindow::reportProgress(int minimum, int value, int maximum, const QString& message)
{
	if (p->m_universalProgress->minimum() != minimum)
		p->m_universalProgress->setMinimum(minimum);
	if (p->m_universalProgress->maximum() != maximum)
		p->m_universalProgress->setMaximum(maximum);
	if (p->m_universalProgress->value() != value)
		p->m_universalProgress->setValue(value);
	p->m_universalProgress->setText(message);
}

void Palapeli::MainWindow::flushProgress()
{
	p->m_universalProgress->reset();
}

#include "mainwindow.moc"
#include "mainwindow_p.moc"

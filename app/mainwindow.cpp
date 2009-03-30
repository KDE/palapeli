/***************************************************************************
 *   Copyright 2008 Felix Lemke <lemke.felix@ages-skripte.org>
 *   Copyright 2008-2009 Stefan Majewsky <majewsky@gmx.net>
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
#include "../lib/core/engine.h"
#include "../lib/core/view.h"
#include "../lib/core/viewmenu.h"
#include "../lib/actions/deleteaction.h"
#include "../lib/actions/exportaction.h"
#include "../lib/actions/importaction.h"
#include "../lib/actions/kns-downloadaction.h"
#include "../lib/actions/loadaction.h"
#include "../lib/actions/resetaction.h"
#include "../lib/actions/textureaction.h"
#include "../lib/library/librarybase.h"
#include "../lib/library/library.h"
#include "../lib/library/puzzleinfo.h"
#include "manager.h"
#include "preview.h"
#include "settings.h"

#include <QCloseEvent>
#include <QTimer>
#include <KActionCollection>
#include <KApplication>
#include <KLocalizedString>
#include <KMenuBar>
#include <KDE/KStandardGameAction>
#include <KStatusBar>
#include <KToggleFullScreenAction>
#include <KToolBar>

Palapeli::MainWindowPrivate::MainWindowPrivate(Palapeli::MainWindow* parent)
	: m_parent(parent)
	, m_togglePreviewAct(new KAction(KIcon("games-config-background"), i18n("Show preview"), parent))
	, m_dockPreview(new QDockWidget(i18n("Image preview"), parent))
	, m_createDialog(0)
	, m_settingsDialog(new KPageDialog(parent))
	, m_appearanceUi(new Ui::AppearanceSettingsWidget)
	, m_appearanceContainer(new QWidget)
	, m_gameplayUi(new Ui::GameplaySettingsWidget)
	, m_gameplayContainer(new QWidget)
	, m_centralWidget(new QStackedWidget)
	, m_welcomeWidget(new Palapeli::WelcomeWidget)
	, m_loaderWidget(new QLabel(i18n("Loading puzzle...")))
{
	m_loaderWidget->setAlignment(Qt::AlignCenter);
	m_centralWidget->addWidget(m_welcomeWidget);
	m_centralWidget->addWidget(m_loaderWidget);
	m_centralWidget->addWidget(ppMgr()->engine()->view());
	m_centralWidget->setCurrentWidget(m_welcomeWidget);
}

Palapeli::MainWindowPrivate::~MainWindowPrivate()
{
	//image preview
	delete m_togglePreviewAct;
	delete m_dockPreview;
	//dialogs
	delete m_createDialog;
	delete m_settingsDialog;
	delete m_appearanceUi;
	delete m_appearanceContainer;
	delete m_gameplayUi;
	delete m_gameplayContainer;
}

void Palapeli::MainWindowPrivate::setupActions()
{
	KAction* action;
	//Game actions
	KStandardGameAction::gameNew(0, "", m_parent->actionCollection());
	action = new Palapeli::LoadAction(m_parent->actionCollection());
	connect(m_welcomeWidget, SIGNAL(libraryRequest()), action, SLOT(trigger()));
	connect(action, SIGNAL(gameLoadRequest(const Palapeli::PuzzleInfo*)), this, SLOT(loadGame(const Palapeli::PuzzleInfo*)));
	action = new Palapeli::ResetAction(m_parent->actionCollection());
	connect(ppMgr(), SIGNAL(gameNameChanged(const QString&)), action, SLOT(gameNameWasChanged(const QString&)));
	connect(action, SIGNAL(reloadGameRequest(const Palapeli::PuzzleInfo*)), this, SLOT(reloadGame(const Palapeli::PuzzleInfo*)));
	action = new Palapeli::ImportAction(m_parent->actionCollection());
	connect(m_welcomeWidget, SIGNAL(importRequest()), action, SLOT(trigger()));
	new Palapeli::ExportAction(m_parent->actionCollection());
	new Palapeli::DeleteAction(m_parent->actionCollection());
	new Palapeli::KnsDownloadAction(m_parent->actionCollection());
	KStandardGameAction::quit(m_parent, SLOT(close()), m_parent->actionCollection());
	//View actions
	m_parent->actionCollection()->addAction("view_toggle_preview", m_togglePreviewAct);
	m_togglePreviewAct->setCheckable(true);
	m_togglePreviewAct->setChecked(false);
	connect(m_dockPreview, SIGNAL(visibilityChanged(bool)), m_togglePreviewAct, SLOT(setChecked(bool)));
	connect(m_togglePreviewAct, SIGNAL(triggered(bool)), m_dockPreview, SLOT(setVisible(bool)));
	KStandardAction::zoomIn(this, SLOT(zoomIn()), m_parent->actionCollection());
	KStandardAction::zoomOut(this, SLOT(zoomOut()), m_parent->actionCollection());
	KStandardAction::fullScreen(this, SLOT(setFullScreen(bool)), m_parent, m_parent->actionCollection());
	//Settings actions
	KStandardAction::preferences(m_settingsDialog, SLOT(show()), m_parent->actionCollection());
	new Palapeli::TextureAction(ppMgr()->engine()->view()->menu(), m_parent->actionCollection());
}

void Palapeli::MainWindowPrivate::setupDockers()
{
	//preview
	m_parent->addDockWidget(Qt::LeftDockWidgetArea, m_dockPreview);
	m_dockPreview->setObjectName("DockPreview");
	m_dockPreview->setWidget(ppMgr()->preview());
	m_dockPreview->resize(1, 1);
	m_dockPreview->setVisible(false); //hidden by default
}

void Palapeli::MainWindowPrivate::setupDialogs()
{
	m_createDialog = new Palapeli::CreateDialog;
	connect(m_parent->actionCollection()->action("game_new"), SIGNAL(triggered()), m_createDialog, SLOT(show()));
	connect(m_welcomeWidget, SIGNAL(createRequest()), m_createDialog, SLOT(show()));
	connect(m_createDialog, SIGNAL(gameCreated(const Palapeli::PuzzleInfo*)), this, SLOT(loadGame(const Palapeli::PuzzleInfo*)));
	//setup Settings UIs
	m_appearanceUi->setupUi(m_appearanceContainer);
	m_gameplayUi->setupUi(m_gameplayContainer);
	m_appearanceUi->checkAntialiasing->setChecked(Settings::antialiasing());
	connect(m_appearanceUi->checkAntialiasing, SIGNAL(stateChanged(int)), this, SLOT(configurationChanged()));
	m_appearanceUi->checkHardwareAccel->setChecked(Settings::hardwareAccel());
	connect(m_appearanceUi->checkHardwareAccel, SIGNAL(stateChanged(int)), this, SLOT(configurationChanged()));
	m_appearanceUi->sceneSizeSlider->setValue(100.0 * Settings::sceneSizeFactor());
	connect(m_appearanceUi->sceneSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(configurationChanged()));
	m_gameplayUi->precisionSlider->setValue(Settings::snappingPrecision());
	connect(m_gameplayUi->precisionSlider, SIGNAL(valueChanged(int)), this, SLOT(configurationChanged()));
#ifndef PALAPELI_WITH_OPENGL
	m_appearanceUi->checkHardwareAccel->setVisible(false);
#endif
	//setup Settings KDialog
	m_settingsDialog->setWindowIcon(KIcon("configure"));
	m_settingsDialog->setCaption(i18n("Configure Palapeli"));
	m_settingsDialog->setButtons(KDialog::Ok | KDialog::Apply | KDialog::Cancel);
	m_settingsDialog->enableButtonApply(false); //not until something has changed
	m_settingsDialog->resize(1, 1); //this lets the dialog scale down to its recommended (i.e. minimum) size
	connect(m_settingsDialog, SIGNAL(okClicked()), this, SLOT(configurationFinished()));
	connect(m_settingsDialog, SIGNAL(applyClicked()), this, SLOT(configurationFinished()));
	//setups Settings KPageDialog
	KPageWidgetItem* appearancePage = new KPageWidgetItem(m_appearanceContainer, i18n("Appearance"));
	appearancePage->setHeader(i18n("Adjust the visual appearance and performance of Palapeli"));
	appearancePage->setIcon(KIcon("games-config-board"));
	m_settingsDialog->addPage(appearancePage);
	KPageWidgetItem* gameplayPage = new KPageWidgetItem(m_gameplayContainer, i18n("Gameplay"));
	gameplayPage->setHeader(i18n("Adjust the gameplay of Palapeli")); //TODO: this is not optimal from my POV
	gameplayPage->setIcon(KIcon("games-config-custom"));
	m_settingsDialog->addPage(gameplayPage);
}

void Palapeli::MainWindowPrivate::configurationChanged() //because of user-invoked changes in the dialog
{
	m_settingsDialog->enableButtonApply(true);
}

void Palapeli::MainWindowPrivate::configurationFinished()
{
	//apply settings
	Palapeli::View* view = ppMgr()->engine()->view();
	view->setAntialiasing(m_appearanceUi->checkAntialiasing->isChecked());
	view->setHardwareAccelerated(m_appearanceUi->checkHardwareAccel->isChecked());
	Settings::setSceneSizeFactor(qreal(m_appearanceUi->sceneSizeSlider->value()) / 100.0);
	Settings::setSnappingPrecision(m_gameplayUi->precisionSlider->value());
	Settings::self()->writeConfig();
	//mark settings as saved in the dialog
	m_settingsDialog->enableButtonApply(false);
}

void Palapeli::MainWindowPrivate::setFullScreen(bool full)
{
	m_parent->menuBar()->setVisible(!full);
	KToggleFullScreenAction::setFullScreen(m_parent, full);
}

void Palapeli::MainWindowPrivate::loadGame(const Palapeli::PuzzleInfo* info)
{
	ppMgr()->loadGame(info); //force reload
}

void Palapeli::MainWindowPrivate::reloadGame(const Palapeli::PuzzleInfo* info)
{
	ppMgr()->loadGame(info, true); //force reload
}

void Palapeli::MainWindowPrivate::zoomIn()
{
	static const qreal scalingFactor = 1.2;
	ppMgr()->engine()->view()->scale(scalingFactor);
}

void Palapeli::MainWindowPrivate::zoomOut()
{
	static const qreal scalingFactor = 1.0 / 1.2;
	ppMgr()->engine()->view()->scale(scalingFactor);
}

Palapeli::MainWindow::MainWindow(QWidget* parent)
	: KXmlGuiWindow(parent)
	, p(new Palapeli::MainWindowPrivate(this))
{
	//initialize actions
	p->setupActions();
	menuBar()->show();
	//early GUI settings
	setAutoSaveSettings();
	setCentralWidget(p->m_centralWidget);
	//initialize dockers
	p->setupDockers();
	//late GUI settings
	setupGUI(QSize(500, 400));
	gameNameWasChanged(QString());
	connect(ppMgr(), SIGNAL(gameNameChanged(const QString&)), this, SLOT(gameNameWasChanged(const QString&)));
	connect(ppMgr(), SIGNAL(interactionModeChanged(bool)), this, SLOT(changeInteractionMode(bool)));
	//menu bar and status bar
	statusBar()->show();
	statusBar()->addPermanentWidget(ppMgr()->engine()->progressBar(), 1);
	//initialise dialogs after entering the event loop (to speed up startup)
	QTimer::singleShot(0, p, SLOT(setupDialogs()));
}

Palapeli::MainWindow::~MainWindow()
{
	delete p;
}

void Palapeli::MainWindow::gameNameWasChanged(const QString& name)
{
	if (name.isEmpty())
		setCaption(i18nc("The application's name", "Palapeli"));
	else
		setCaption(i18nc("For usage as window title", "%1 - Palapeli", name));
}

void Palapeli::MainWindow::changeInteractionMode(bool allowGameInteraction)
{
	actionCollection()->action(QLatin1String("palapeli_load"))->setEnabled(allowGameInteraction);
	p->m_centralWidget->setCurrentWidget(allowGameInteraction ? (QWidget*) ppMgr()->engine()->view() : (QWidget*) p->m_loaderWidget);
}

void Palapeli::MainWindow::closeEvent(QCloseEvent* event)
{
	event->accept();
	kapp->quit();
}

#include "mainwindow.moc"
#include "mainwindow_p.moc"

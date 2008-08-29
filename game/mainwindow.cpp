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
#include "autosaver.h"
#include "interface/exportwidget.h"
#include "interface/importwidget.h"
#include "interface/loadwidget.h"
#include "interface/savewidget.h"
#include "manager.h"
#include "minimap.h"
#include "preview.h"
#include "settings.h"
#include "view.h"

#include <QCloseEvent>
#include <QTimer>
#include <KActionCollection>
#include <KLocalizedString>
#include <KMenuBar>
#include <KDE/KStandardGameAction>
#include <KStatusBar>
#include <KToggleFullScreenAction>

Palapeli::MainWindowPrivate::MainWindowPrivate(Palapeli::MainWindow* parent)
	: m_parent(parent)
	, m_toggleMinimapAct(new KAction(KIcon("document-preview"), i18n("Show minimap"), parent))
	, m_togglePreviewAct(new KAction(KIcon("games-config-background"), i18n("Show preview"), parent))
	, m_dockMinimap(new QDockWidget(i18n("Overview"), parent))
	, m_dockPreview(new QDockWidget(i18n("Image preview"), parent))
	, m_newDialog(0) //cannot be created until Manager has loaded the pattern plugins
	, m_settingsDialog(new KPageDialog(parent))
	, m_appearanceUi(new Ui::AppearanceSettingsWidget)
	, m_appearanceContainer(new QWidget)
	, m_gameplayUi(new Ui::GameplaySettingsWidget)
	, m_gameplayContainer(new QWidget)
	, m_puzzleProgress(new Palapeli::TextProgressBar)
	, m_universalProgress(new Palapeli::TextProgressBar)
{
}

Palapeli::MainWindowPrivate::~MainWindowPrivate()
{
	//actions
	delete m_toggleMinimapAct;
	delete m_togglePreviewAct;
	//docker widgets
	delete m_dockMinimap;
	delete m_dockPreview;
	//dialogs
	delete m_newDialog;
	delete m_settingsDialog;
	delete m_appearanceUi;
	delete m_appearanceContainer;
	delete m_gameplayUi;
	delete m_gameplayContainer;
	//status bar
	delete m_puzzleProgress;
	delete m_universalProgress;
}

void Palapeli::MainWindowPrivate::setupActions()
{
	//Game actions
	KStandardGameAction::gameNew(0, "", m_parent->actionCollection()); //no slot given because m_newDialog is not available yet
	new Palapeli::LoadWidgetAction(m_parent->actionCollection());
	new Palapeli::SaveWidgetAction(m_parent->actionCollection());
	new Palapeli::ImportWidgetAction(m_parent->actionCollection());
	new Palapeli::ExportWidgetAction(m_parent->actionCollection());
	KStandardGameAction::quit(m_parent, SLOT(close()), m_parent->actionCollection());
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
	KStandardAction::fullScreen(this, SLOT(setFullScreen(bool)), m_parent, m_parent->actionCollection());
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
}

void Palapeli::MainWindowPrivate::setupDialogs()
{
	//setup "New game" dialog - it is now safe to do that because Manager has loaded its pattern plugins
	m_newDialog = new Palapeli::NewPuzzleDialog(m_parent);
	connect(m_parent->actionCollection()->action(KStandardGameAction::name(KStandardGameAction::New)), SIGNAL(triggered()), m_newDialog, SLOT(showDialog()));
	connect(m_newDialog, SIGNAL(startGame(const KUrl&, int)), ppMgr(), SLOT(createGame(const KUrl&, int)));
	connect(m_newDialog, SIGNAL(startGame(const QString&)), ppMgr(), SLOT(createGame(const QString&)));
	//setup Settings UIs
	m_appearanceUi->setupUi(m_appearanceContainer);
	m_gameplayUi->setupUi(m_gameplayContainer);
	m_appearanceUi->checkAntialiasing->setChecked(Settings::antialiasing());
	connect(m_appearanceUi->checkAntialiasing, SIGNAL(stateChanged(int)), this, SLOT(configurationChanged()));
	m_appearanceUi->checkHardwareAccel->setChecked(Settings::hardwareAccel());
	connect(m_appearanceUi->checkHardwareAccel, SIGNAL(stateChanged(int)), this, SLOT(configurationChanged()));
	m_appearanceUi->sceneSizeSlider->setValue(100.0 * Settings::sceneSizeFactor());
	connect(m_appearanceUi->sceneSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(configurationChanged()));
	m_appearanceUi->checkMinimapQuality->setChecked(Settings::minimapQuality());
	connect(m_appearanceUi->checkMinimapQuality, SIGNAL(stateChanged(int)), this, SLOT(configurationChanged()));
	m_gameplayUi->precisionSlider->setValue(Settings::snappingPrecision());
	connect(m_gameplayUi->precisionSlider, SIGNAL(valueChanged(int)), this, SLOT(configurationChanged()));
#ifndef PALAPELI_WITH_OPENGL
	m_appearanceUi->checkHardwareAccel->setVisible(false);
#endif
	connect(m_gameplayUi->radioAutosaveTime, SIGNAL(toggled(bool)), this, SLOT(configurationChanged()));
	connect(m_gameplayUi->spinAutosaveTime, SIGNAL(valueChanged(int)), this, SLOT(configurationChanged()));
	if (Settings::autosaveTime() != 0)
	{
		m_gameplayUi->radioAutosaveTime->setChecked(true);
		m_gameplayUi->spinAutosaveTime->setValue(Settings::autosaveTime());
	}
	connect(m_gameplayUi->radioAutosaveMove, SIGNAL(toggled(bool)), this, SLOT(configurationChanged()));
	connect(m_gameplayUi->spinAutosaveMove, SIGNAL(valueChanged(int)), this, SLOT(configurationChanged()));
	if (Settings::autosaveMoves() != 0)
	{
		m_gameplayUi->radioAutosaveMove->setChecked(true);
		m_gameplayUi->spinAutosaveMove->setValue(Settings::autosaveMoves());
	}
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
	ppMgr()->view()->setAntialiasing(m_appearanceUi->checkAntialiasing->isChecked());
	ppMgr()->view()->setHardwareAccelerated(m_appearanceUi->checkHardwareAccel->isChecked());
	ppMgr()->minimap()->setQualityLevel(m_appearanceUi->checkMinimapQuality->isChecked() ? 1 : 0);
	if (m_gameplayUi->radioAutosaveTime->isChecked())
		ppMgr()->autosaver()->setTimeInterval(m_gameplayUi->spinAutosaveTime->value());
	else
		ppMgr()->autosaver()->setTimeInterval(0);
	if (m_gameplayUi->radioAutosaveMove->isChecked())
		ppMgr()->autosaver()->setMoveInterval(m_gameplayUi->spinAutosaveMove->value());
	else
		ppMgr()->autosaver()->setMoveInterval(0);
	Settings::setSceneSizeFactor(qreal(m_appearanceUi->sceneSizeSlider->value()) / 100.0);
	Settings::setSnappingPrecision(m_gameplayUi->precisionSlider->value());
	Settings::self()->writeConfig();
	//mark settings as saved in the dialog
	m_settingsDialog->enableButtonApply(false);
	//report progress
	m_parent->reportProgress(0, 1, 1, i18n("Settings saved."));
	m_parent->flushProgress(2);
}

void Palapeli::MainWindowPrivate::setFullScreen(bool full)
{
	m_parent->menuBar()->setVisible(!full);
	KToggleFullScreenAction::setFullScreen(m_parent, full);
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
	gameNameWasChanged(QString());
	connect(ppMgr(), SIGNAL(gameNameChanged(const QString&)), this, SLOT(gameNameWasChanged(const QString&)));
	connect(ppMgr(), SIGNAL(interactionModeChanged(bool)), this, SLOT(changeInteractionMode(bool)));
	//menu bar and status bar
	menuBar()->show();
	statusBar()->show();
	statusBar()->addPermanentWidget(p->m_universalProgress, 1);
	statusBar()->addPermanentWidget(p->m_puzzleProgress, 1);
	p->m_universalProgress->setText(i18n("Welcome to Palapeli."));
	p->m_puzzleProgress->setText(i18n("Click \"New\" to start a new puzzle game."));
	p->m_universalProgress->flush(5);
	//initialise dialogs after entering the event loop (to speed up startup)
	QTimer::singleShot(0, p, SLOT(setupDialogs()));
}

Palapeli::MainWindow::~MainWindow()
{
	delete p;
}

void Palapeli::MainWindow::reportPuzzleProgress(int pieceCount, int partCount)
{
	if (p->m_puzzleProgress->minimum() != 0)
		p->m_puzzleProgress->setMinimum(0);
	if (p->m_puzzleProgress->maximum() != pieceCount - 1)
		p->m_puzzleProgress->setMaximum(pieceCount - 1);
	const int value = pieceCount - partCount;
	if (p->m_puzzleProgress->value() != value)
	{
		p->m_puzzleProgress->setValue(value);
		if (partCount == 1)
			p->m_puzzleProgress->setText(i18n("You finished the puzzle."));
		else
		{
			int percentFinished = qreal(value) / qreal(pieceCount - 1) * 100;
			p->m_puzzleProgress->setText(i18n("%1% finished", percentFinished));
		}
	}
}

void Palapeli::MainWindow::flushPuzzleProgress()
{
	p->m_universalProgress->flush(0);
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

void Palapeli::MainWindow::flushProgress(int secondsDelay)
{
	p->m_universalProgress->flush(secondsDelay);
}

void Palapeli::MainWindow::gameNameWasChanged(const QString& name)
{
	if (name.isEmpty())
		setCaption(i18nc("The application's name", "Palapeli"));
	else
		setCaption(i18nc("For usage as window title", "%1 - Palapeli").arg(name));
}

void Palapeli::MainWindow::changeInteractionMode(bool allowGameInteraction)
{
	actionCollection()->action(KStandardGameAction::name(KStandardGameAction::New))->setEnabled(allowGameInteraction);
	actionCollection()->action(QLatin1String("palapeli_load"))->setEnabled(allowGameInteraction);
	actionCollection()->action(QLatin1String("palapeli_save"))->setEnabled(allowGameInteraction);
	actionCollection()->action(QLatin1String("palapeli_import"))->setEnabled(allowGameInteraction);
	actionCollection()->action(QLatin1String("palapeli_export"))->setEnabled(allowGameInteraction);
}

void Palapeli::MainWindow::closeEvent(QCloseEvent* event)
{
	ppMgr()->ensurePersistence(Palapeli::Manager::ClosingApp) ? event->accept() : event->ignore();
}

#include "mainwindow.moc"
#include "mainwindow_p.moc"

/***************************************************************************
 *   Copyright 2009, 2010 Stefan Majewsky <majewsky@gmx.net>
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
#include "../config/configdialog.h"
#include "../creator/puzzlecreator.h"
#include "../engine/interactormanager.h"
#include "../engine/scene.h"
#include "../engine/texturehelper.h"
#include "../engine/view.h"
#include "collectionwidget.h"
#include "puzzletablewidget.h"
#include "settings.h"
#include "tabwindow.h"
#include "ui_settings.h"

#include <QPointer>
#include <QTabBar> //krazy:exclude=qclasses
#include <KActionCollection>
#include <KCmdLineArgs>
#include <KConfigDialog>
#include <KLocalizedString>
#include <KMenuBar>
#include <KTabWidget>
#include <KToggleAction>
#include <KShortcutsDialog>
#include <KStandardAction>

namespace Palapeli
{
	class KTabWidget : public ::KTabWidget
	{
		public:
			QTabBar* tabBar() const { return ::KTabWidget::tabBar(); } //krazy:exclude=qclasses
	};
}

Palapeli::MainWindow::MainWindow(KCmdLineArgs* args)
	: m_centralWidget(new Palapeli::KTabWidget)
	, m_collectionWidget(new Palapeli::CollectionWidget)
	, m_puzzleTable(new Palapeli::PuzzleTableWidget)
{
	//create MainWindow-wide actions
	KStandardAction::keyBindings(this, SLOT(configureShortcuts()), actionCollection());
	KStandardAction::preferences(this, SLOT(configurePalapeli()), actionCollection());
	KAction* statusBarAct = KStandardAction::showStatusbar(m_puzzleTable, SLOT(showStatusBar(bool)), actionCollection());
	statusBarAct->setText(i18n("Show statusbar of puzzle table"));
	//read settings
	bool showStatusBar = Settings::showStatusBar();
	statusBarAct->setChecked(showStatusBar);
	m_puzzleTable->showStatusBar(showStatusBar);
	//setup widgets
	m_centralWidget->addTab(m_collectionWidget, i18n("My collection"));
	m_centralWidget->addTab(m_puzzleTable, i18n("Puzzle table"));
	m_centralWidget->setTabEnabled(m_centralWidget->indexOf(m_puzzleTable), false); //... until a puzzle has been loaded
	m_centralWidget->setCurrentWidget(m_collectionWidget);
	connect(m_collectionWidget, SIGNAL(createRequest()), this, SLOT(createPuzzle()));
	connect(m_collectionWidget, SIGNAL(playRequest(const QModelIndex&)), this, SLOT(loadPuzzle(const QModelIndex&)));
	//setup main window
	setCentralWidget(m_centralWidget);
	KXmlGuiWindow::StandardWindowOptions guiOptions = KXmlGuiWindow::Default;
	guiOptions &= ~KXmlGuiWindow::StatusBar; //do not create a statusbar for the mainwindow
	guiOptions &= ~KXmlGuiWindow::Keys;      //Palapeli has its own shortcuts dialog
	guiOptions &= ~KXmlGuiWindow::ToolBar;   //I haven't yet found a way for KEditToolBar dialogs to work
	setupGUI(QSize(500, 500), guiOptions);
	//move the menubar inside the tabbar (to make the tabs feel like menus) - Unfortunately, we can't use QTabWidget::setCornerWidget because this would move the menubar to the right end of the window, while I want the menubar right next to the tabs. We therefore have to do our own layouting (and remove the menubar from the window's layout with reparenting)
	m_menuBar = menuBar();
	m_menuBar->QWidget::setParent(0);
	m_menuBar->QWidget::setParent(this);
	m_menuBar->raise();
	doMenuLayout();
	//start a puzzle if a puzzle URL has been given
	if (args->count() > 0)
	{
		m_collectionWidget->startPuzzle(args->url(0));
		m_centralWidget->setTabEnabled(m_centralWidget->indexOf(m_collectionWidget), false);
		m_centralWidget->setTabEnabled(m_centralWidget->indexOf(m_puzzleTable), true);
		m_centralWidget->setCurrentWidget(m_puzzleTable);
	}
	args->clear();
}

void Palapeli::MainWindow::doMenuLayout()
{
	//determine geometry of menubar...
	QRect rect = this->rect();
	const QSize tabBarSize = m_centralWidget->tabBar()->sizeHint();
	const QSize menuBarSize = m_menuBar->sizeHint();
	setMinimumWidth(tabBarSize.width());
	//...in X direction
	if (QApplication::isLeftToRight())
		rect.setLeft(tabBarSize.width());
	else
		rect.setRight(rect.width() - tabBarSize.width());
	//...in Y direction
	const int height = menuBarSize.height();
	const int maxHeight = tabBarSize.height() - style()->pixelMetric(QStyle::PM_TabBarBaseHeight, 0, this);
	const int yPos = (maxHeight - height) / 2; //vertical alignment on tab bar
	rect.setHeight(height);
	rect.moveTop(qMax(yPos, 0)); //do not allow yPos < 0!
	//done
	m_menuBar->setGeometry(rect);
}

void Palapeli::MainWindow::changeEvent(QEvent* event)
{
	KXmlGuiWindow::changeEvent(event);
	switch (event->type())
	{
		case QEvent::FontChange:
		case QEvent::LanguageChange:
		case QEvent::LayoutDirectionChange:
		case QEvent::LocaleChange:
		case QEvent::StyleChange:
			//relayout the menu whenever the tabbar size may have changed
			doMenuLayout();
			break;
		default:
			break;
	}
}

void Palapeli::MainWindow::resizeEvent(QResizeEvent* event)
{
	KXmlGuiWindow::resizeEvent(event);
	doMenuLayout();
}

void Palapeli::MainWindow::createPuzzle()
{
	QPointer<Palapeli::PuzzleCreatorDialog> creatorDialog(new Palapeli::PuzzleCreatorDialog);
	if (creatorDialog->exec())
	{
		if (!creatorDialog)
			return;
		Palapeli::Puzzle* puzzle = creatorDialog->result();
		if (!puzzle)
			return;
		QModelIndex index = m_collectionWidget->storeGeneratedPuzzle(puzzle);
		if (index.isValid())
			loadPuzzle(index);
	}
	delete creatorDialog;
}

void Palapeli::MainWindow::loadPuzzle(const QModelIndex& index)
{
	m_puzzleTable->view()->scene()->loadPuzzle(index);
	m_centralWidget->setTabEnabled(m_centralWidget->indexOf(m_puzzleTable), true);
	m_centralWidget->setCurrentWidget(m_puzzleTable);
	setCaption(index.data(Qt::DisplayRole).toString());
}

void Palapeli::MainWindow::configureShortcuts()
{
	KShortcutsDialog dlg(KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsAllowed, this);
	dlg.addCollection(m_collectionWidget->actionCollection());
	dlg.addCollection(m_puzzleTable->actionCollection());
	dlg.configure(true);
}

//TODO: Palapeli::MainWindow::configureToolbars

void Palapeli::MainWindow::configurePalapeli()
{
	Palapeli::ConfigDialog dialog(m_puzzleTable->view());
	dialog.exec();
#if 0
	//setup "General settings" widget
	QWidget* settingsWidget = new QWidget;
	Ui::Settings settingsUi; settingsUi.setupUi(settingsWidget);
	//NOTE: It is intentional that the widget "cfg_ViewBackground" is _not_ called "kcfg_ViewBackground". KConfigDialog's config handling would mess things up if it was used in this case.
	settingsUi.cfg_ViewBackground->setModel(m_puzzleTable->view()->textureHelper());
	connect(settingsUi.cfg_ViewBackground, SIGNAL(currentIndexChanged(int)), this, SLOT(configure_TextureChanged(int)));
	connect(this, SIGNAL(configure_ColorEnabledChanged(bool)), settingsUi.kcfg_ViewBackgroundColor, SLOT(setEnabled(bool)));
	settingsUi.cfg_ViewBackground->setCurrentIndex(m_puzzleTable->view()->textureHelper()->currentIndex());
	connect(settingsUi.cfg_ViewBackground, SIGNAL(currentIndexChanged(int)), m_puzzleTable->view()->textureHelper(), SLOT(setCurrentIndex(int)));
	connect(settingsUi.kcfg_ViewBackgroundColor, SIGNAL(changed(QColor)), m_puzzleTable->view()->textureHelper(), SLOT(setSolidColor(QColor)));
	//FIXME: Both TextureHelper and InteractorManager immediately apply changes made to their configuration through the UI, which is not what the user expects.
	//setup dialog
	KConfigDialog settingsDialog(this, QString(), Settings::self());
	settingsDialog.addPage(settingsWidget, i18n("General settings"))->setIcon(KIcon("configure"));
	settingsDialog.addPage(new Palapeli::TriggerConfigWidget(m_puzzleTable->view()->interactorManager()), i18n("Mouse interaction"))->setIcon(KIcon("input-mouse"));
// 	connect(&settingsDialog, SIGNAL(settingsChanged(const QString&)), this, SLOT(configureFinished())); //NOTE: unused ATM (settings are read on demand)
	settingsDialog.exec();
}

void Palapeli::MainWindow::configure_TextureChanged(int index)
{
	QComboBox* backgroundBox = qobject_cast<QComboBox*>(sender()); //krazy:exclude=qclasses
	if (!backgroundBox)
		return;
	const QString selectedStyle = backgroundBox->itemData(index, Palapeli::TextureHelper::StyleRole).toString();
	emit configure_ColorEnabledChanged(selectedStyle == "color");
#endif
}

#include "mainwindow.moc"

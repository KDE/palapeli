/***************************************************************************
 *   Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
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
#include "../creator/puzzlecreator.h"
#include "../engine/scene.h"
#include "../engine/texturehelper.h"
#include "../engine/view.h"
#include "librarywidget.h"
#include "puzzletablewidget.h"
#include "settings.h"
#include "tabwindow.h"
#include "ui_settings.h"

#include <QPointer>
#include <QTabBar> //krazy:exclude=qclasses
#include <KActionCollection>
#include <KConfigDialog>
#include <KLocalizedString>
#include <KMenuBar>
#include <KTabWidget>
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

Palapeli::MainWindow::MainWindow()
	: m_centralWidget(new Palapeli::KTabWidget)
	, m_library(new Palapeli::LibraryWidget)
	, m_puzzleTable(new Palapeli::PuzzleTableWidget)
{
	//create MainWindow-wide actions
	KStandardAction::keyBindings(this, SLOT(configureShortcuts()), actionCollection());
	KStandardAction::preferences(this, SLOT(configurePalapeli()), actionCollection());
	//setup widgets
	m_centralWidget->addTab(m_library, i18n("My library"));
	m_centralWidget->addTab(m_puzzleTable, i18n("Puzzle table"));
	m_centralWidget->setTabEnabled(m_centralWidget->indexOf(m_puzzleTable), false); //... until a puzzle has been loaded
	m_centralWidget->setCurrentWidget(m_library);
	connect(m_library, SIGNAL(createRequest()), this, SLOT(createPuzzle()));
	connect(m_library, SIGNAL(playRequest(const QModelIndex&)), this, SLOT(loadPuzzle(const QModelIndex&)));
	//setup main window
	setCentralWidget(m_centralWidget);
	KXmlGuiWindow::StandardWindowOptions guiOptions = KXmlGuiWindow::Default;
	guiOptions &= ~KXmlGuiWindow::StatusBar; //do not create statusbar
	guiOptions &= ~KXmlGuiWindow::Keys;      //Palapeli has our own shortcuts dialog
	guiOptions &= ~KXmlGuiWindow::ToolBar;   //I haven't yet found a way for KEditToolBar dialogs to work
	setupGUI(QSize(500, 500), guiOptions);
	//move the menubar inside the tabbar (to make the tabs feel like menus) - Unfortunately, we can't use QTabWidget::setCornerWidget because this would move the menubar to the right end of the window, while I want the menubar right next to the tabs. We therefore have to do our own layouting (and remove the menubar from the window's layout with reparenting)
	m_menuBar = menuBar();
	m_menuBar->QWidget::setParent(0);
	m_menuBar->QWidget::setParent(this);
	m_menuBar->raise();
	setMinimumWidth(m_menuBar->sizeHint().width() + m_centralWidget->tabBar()->sizeHint().width());
}

void Palapeli::MainWindow::resizeEvent(QResizeEvent* event)
{
	KXmlGuiWindow::resizeEvent(event);
	//determine geometry of menubar...
	QRect rect = this->rect();
	const QSize tabBarSize = m_centralWidget->tabBar()->sizeHint();
	const QSize menuBarSize = m_menuBar->sizeHint();
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
		QModelIndex index = m_library->storeGeneratedPuzzle(puzzle);
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
}

void Palapeli::MainWindow::configureShortcuts()
{
	KShortcutsDialog dlg(KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsAllowed, this);
	dlg.addCollection(m_library->actionCollection());
	dlg.addCollection(m_puzzleTable->actionCollection());
	dlg.configure(true);
}

//TODO: Palapeli::MainWindow::configureToolbars

void Palapeli::MainWindow::configurePalapeli()
{
	//setup settings widget
	QWidget* settingsWidget = new QWidget;
	Ui::Settings settingsUi; settingsUi.setupUi(settingsWidget);
	//NOTE: It is intentional that the widget "cfg_ViewBackground" is _not_ called "kcfg_ViewBackground". KConfigDialog's config handling would mess things up if it was used in this case.
	settingsUi.cfg_ViewBackground->setModel(m_puzzleTable->view()->textureHelper());
	settingsUi.cfg_ViewBackground->setCurrentIndex(m_puzzleTable->view()->textureHelper()->currentIndex());
	connect(settingsUi.cfg_ViewBackground, SIGNAL(currentIndexChanged(int)), m_puzzleTable->view()->textureHelper(), SLOT(setCurrentIndex(int)));
	//setup dialog
	KConfigDialog settingsDialog(this, QString(), Settings::self());
	settingsDialog.addPage(settingsWidget, i18n("General settings"))->setIcon(KIcon("configure"));
	//NOTE: no need to connect KConfigDialog::settingsChanged(const QString&) because settings are read on demand
	settingsDialog.setFaceType(KPageDialog::Plain);
	settingsDialog.exec();
}

#include "mainwindow.moc"

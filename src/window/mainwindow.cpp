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
#include "../engine/scene.h"
#include "../engine/view.h"
#include "../file-io/librarymodel.h"
#include "../file-io/libraryview.h"
#include "librarywidget.h"
#include "puzzletablewidget.h"
#include "settings.h"
#include "tabwindow.h"
#include "ui_settings.h"

#include <QTabBar>
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
			QTabBar* tabBar() const { return ::KTabWidget::tabBar(); }
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
	m_centralWidget->setCurrentWidget(m_library);
	connect(m_library->view(), SIGNAL(selected(Palapeli::PuzzleReader*)), this, SLOT(loadPuzzle(Palapeli::PuzzleReader*)));
	//setup main window
	setCentralWidget(m_centralWidget);
	KXmlGuiWindow::StandardWindowOptions guiOptions = KXmlGuiWindow::Default;
	guiOptions &= ~KXmlGuiWindow::StatusBar; //do not create statusbar
	guiOptions &= ~KXmlGuiWindow::Keys;      //Palapeli has our own shortcuts dialog
	guiOptions &= ~KXmlGuiWindow::ToolBar;   //I haven't yet found a way for KEditToolBar dialogs to work
	setupGUI(QSize(600, 400), guiOptions);
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
	rect.moveLeft(tabBarSize.width());
	//...in Y direction
	const int height = menuBarSize.height();
	const int maxHeight = tabBarSize.height() - style()->pixelMetric(QStyle::PM_TabBarBaseHeight, 0, this);
	const int yPos = (maxHeight - height) / 2; //vertical alignment on tab bar
	rect.setHeight(height);
	rect.moveTop(qMax(yPos, 0)); //do not allow yPos < 0!
	//done
	m_menuBar->setGeometry(rect);
}

void Palapeli::MainWindow::showEvent(QShowEvent* event)
{
	//if a puzzle file has been given via the CLI, load that puzzle (see LibraryModel constructor for details)
	QModelIndex index = m_library->view()->model()->index(0);
	if (index.data(Palapeli::LibraryModel::IsFromLibraryRole) == QVariant(false))
		loadPuzzle(m_library->view()->model()->puzzle(index));
}

void Palapeli::MainWindow::configureShortcuts()
{
	KShortcutsDialog dlg(KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsAllowed, this);
	dlg.addCollection(m_library->actionCollection());
	dlg.addCollection(m_puzzleTable->actionCollection());
	dlg.configure(true);
}

void Palapeli::MainWindow::loadPuzzle(Palapeli::PuzzleReader* puzzle)
{
	m_puzzleTable->view()->scene()->loadPuzzle(puzzle);
	m_centralWidget->setCurrentWidget(m_puzzleTable);
}

void Palapeli::MainWindow::configurePalapeli()
{
	KConfigDialog settingsDialog(this, QString(), Settings::self());
	QWidget* settingsWidget = new QWidget;
	Ui::Settings settingsUi; settingsUi.setupUi(settingsWidget);
	settingsDialog.addPage(settingsWidget, i18n("General settings"))->setIcon(KIcon("configure"));
	//NOTE: no need to connect KConfigDialog::settingsChanged(const QString&) because settings are read on demand
	settingsDialog.setFaceType(KPageDialog::Plain);
	settingsDialog.exec();
}

#include "mainwindow.moc"

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
#include "../file-io/libraryview.h"
#include "librarywidget.h"
#include "puzzletablewidget.h"
#include "settings.h"
#include "tabwindow.h"
#include "ui_settings.h"

#include <KActionCollection>
#include <KConfigDialog>
#include <KLocalizedString>
#include <KMenuBar>
#include <KTabWidget>
#include <KShortcutsDialog>
#include <KStandardAction>

Palapeli::MainWindow::MainWindow()
	: m_centralWidget(new KTabWidget)
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
	//move the menubar inside the tabbar (to make the tabs feel like menus)
	m_centralWidget->setCornerWidget(menuBar(), Qt::TopRightCorner);
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

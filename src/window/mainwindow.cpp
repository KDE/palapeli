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
#include "../file-io/libraryview.h"
#include "puzzletablewidget.h"
#include "tabwindow.h"

#include <KLocalizedString>
#include <KStatusBar>
#include <KTabWidget>

Palapeli::MainWindow::MainWindow()
	: m_centralWidget(new KTabWidget)
	, m_puzzleTable(new Palapeli::PuzzleTableWidget)
	, m_library(new Palapeli::LibraryView)
{
	//NOTE: create MainWindow-wide actions here
	//setup widgets
	m_centralWidget->addTab(m_library, i18n("My library"));
	m_centralWidget->addTab(m_puzzleTable, i18n("Puzzle table"));
	m_centralWidget->setCurrentWidget(m_library);
	connect(m_library, SIGNAL(selected(Palapeli::PuzzleReader*)), this, SLOT(loadPuzzle(Palapeli::PuzzleReader*)));
	//setup main window
	setCentralWidget(m_centralWidget);
	KXmlGuiWindow::StandardWindowOptions guiOptions = KXmlGuiWindow::Default;
	guiOptions &= ~KXmlGuiWindow::StatusBar; //do not create statusbar
	setupGUI(QSize(600, 400), guiOptions);
// 	statusBar()->hide(); //statusbar is not used in Palapeli
}

void Palapeli::MainWindow::loadPuzzle(Palapeli::PuzzleReader* puzzle)
{
	m_puzzleTable->loadPuzzle(puzzle);
	m_centralWidget->setCurrentWidget(m_puzzleTable);
}

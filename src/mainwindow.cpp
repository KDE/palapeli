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
#include "minimap.h"
#include "preview.h"
#include "scene.h"
#include "view.h"

#include <KActionCollection>
#include <KApplication>
#include <KLocalizedString>
#include <KStandardGameAction>

#include <QDockWidget>

Palapeli::MainWindow::MainWindow(int sceneWidth, int sceneHeight, const QString &fileName, int xPieces, int yPieces, QWidget* parent)
	: KXmlGuiWindow(parent)
	, m_dockmap(new QDockWidget(i18n("Minimap")))
	, m_minimap(new Palapeli::Minimap)
	, m_dockpreview(new QDockWidget(i18n("Preview")))
	, m_preview(new Palapeli::Preview)
	, m_sceneWidth(sceneWidth)
	, m_sceneHeight(sceneHeight)
	, m_fileName(fileName)
	, m_xPieces(xPieces)
	, m_yPieces(yPieces)
	, m_view(new Palapeli::View)
{
	//Game actions
	KStandardGameAction::gameNew(this, SLOT(startGame()), actionCollection());
	KStandardGameAction::quit(kapp, SLOT(quit()), actionCollection());
	//gui settings
	setAutoSaveSettings();
	setCentralWidget(m_view);
	//minimap
	addDockWidget(Qt::RightDockWidgetArea, m_dockmap);
	m_dockmap->setObjectName("DockMap");
	m_dockmap->setWidget(m_minimap);
	//preview
	addDockWidget(Qt::RightDockWidgetArea, m_dockpreview);
	m_dockpreview->setObjectName("DockPreview");
	m_dockpreview->setWidget(m_preview);
	//late settings
	setupGUI(QSize(400, 400));
	setCaption(i18nc("The application's name", "Palapeli"));
}

Palapeli::MainWindow::~MainWindow()
{
}

void Palapeli::MainWindow::startGame()
{
	m_view->startGame(m_sceneWidth, m_sceneHeight, m_fileName, m_xPieces, m_yPieces);
	m_minimap->setView(m_view);
	m_preview->loadImage(m_fileName);
}

#include "mainwindow.moc"

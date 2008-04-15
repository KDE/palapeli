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
#include "ui_dialognew.h"
#include "view.h"

#include <KActionCollection>
#include <KApplication>
#include <KDialog>
#include <KLocalizedString>
#include <KMessageBox>
#include <KDE/KStandardGameAction>
#include <kio/netaccess.h>

#include <QDockWidget>
#include <QTimer>

Palapeli::MainWindow::MainWindow(QWidget* parent)
	: KXmlGuiWindow(parent)
	, m_view(new Palapeli::View)
	, m_dockmap(new QDockWidget(i18n("Overview")))
	, m_minimap(new Palapeli::Minimap)
	, m_dockpreview(new QDockWidget(i18n("Image preview")))
	, m_preview(new Palapeli::Preview)
	, m_newDialog(new KDialog(this))
	, m_newUi(new Ui::NewPuzzleDialog)
{
	//Game actions
	KStandardGameAction::gameNew(m_newDialog, SLOT(show()), actionCollection());
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
	//initialise dialogs after entering the event loop (to speed up startup)
	QTimer::singleShot(0, this, SLOT(setupDialogs()));
}

void Palapeli::MainWindow::setupDialogs()
{
	//setup UI
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
	//setup dialog
	m_newDialog->setCaption(i18n("New puzzle"));
	m_newDialog->setButtons(KDialog::Ok | KDialog::Cancel);
	m_newDialog->mainWidget()->layout()->setMargin(0);
	connect(m_newDialog, SIGNAL(okClicked()), this, SLOT(newGame()));
}

Palapeli::MainWindow::~MainWindow()
{
}

void Palapeli::MainWindow::newGame()
{
	//translate given image URL into local file
	KUrl imageUrl = m_newUi->urlImage->url();
	QString imageFile;
	if(!KIO::NetAccess::download(imageUrl, imageFile, this))
	{
		KMessageBox::error(this, KIO::NetAccess::lastErrorString());
		return;
	}
	//start game
	m_view->startGame(-1, -1, imageFile, m_newUi->spinHorizontalPieces->value(), m_newUi->spinVerticalPieces->value());
	m_preview->loadImage(imageFile);
	m_minimap->setView(m_view);
	//cleanup temporary files
	KIO::NetAccess::removeTempFile(imageFile);
}

#include "mainwindow.moc"

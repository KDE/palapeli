/***************************************************************************
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
#include "action-addrelation.h"
#include "action-removerelation.h"
#include "manager.h"
#include "objview.h"
#include "points.h"
#include "propmodel.h"
#include "shapes.h"
#include "view.h"

#include <QDockWidget>
#include <QTableView>
#include <KAction>
#include <KActionCollection>
#include <KFileDialog>
#include <KIO/NetAccess>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStatusBar>

Paladesign::MainWindow::MainWindow(Paladesign::Manager* manager)
	: KXmlGuiWindow()
	, m_manager(manager)
	, m_objectDock(new QDockWidget(i18n("Objects"), this))
	, m_toggleObjectDockAct(new KAction(i18n("Objects"), this))
	, m_propertyDock(new QDockWidget(i18n("Properties"), this))
	, m_togglePropertyDockAct(new KAction(i18n("Properties"), this))
	, m_propertyView(new QTableView(m_propertyDock))
	, m_selectShapeAct(new KAction(KIcon("insert-image"), i18n("Select shape"), this))
	, m_addRelationAct(new Paladesign::AddRelationAction(manager))
	, m_removeRelationAct(new Paladesign::RemoveRelationAction(manager))
{
	//docker actionsß
	actionCollection()->addAction("dock_objects", m_toggleObjectDockAct);
	m_toggleObjectDockAct->setCheckable(true);
	m_toggleObjectDockAct->setChecked(false);
	connect(m_objectDock, SIGNAL(visibilityChanged(bool)), m_toggleObjectDockAct, SLOT(setChecked(bool)));
	connect(m_toggleObjectDockAct, SIGNAL(triggered(bool)), m_objectDock, SLOT(setVisible(bool)));
	actionCollection()->addAction("dock_properties", m_togglePropertyDockAct);
	m_togglePropertyDockAct->setCheckable(true);
	m_togglePropertyDockAct->setChecked(false);
	connect(m_propertyDock, SIGNAL(visibilityChanged(bool)), m_togglePropertyDockAct, SLOT(setChecked(bool)));
	connect(m_togglePropertyDockAct, SIGNAL(triggered(bool)), m_propertyDock, SLOT(setVisible(bool)));
	//editor actions
	actionCollection()->addAction("editor_selectshape", m_selectShapeAct);
	connect(m_selectShapeAct, SIGNAL(triggered()), this, SLOT(selectShape()));
	actionCollection()->addAction("editor_addrelation", m_addRelationAct);
	actionCollection()->addAction("editor_removerelation", m_removeRelationAct);
	connect(m_manager->objectView(), SIGNAL(selected(QObject*)), m_removeRelationAct, SLOT(selectedRelationChanged()), Qt::QueuedConnection); //queued connection because the RemoveRelationAction requires that the selection changes have already been applied to the relations
	//early GUI settings
	setAutoSaveSettings();
	setCentralWidget(m_manager->view());
	//dock widgets
	addDockWidget(Qt::RightDockWidgetArea, m_objectDock);
	m_objectDock->setObjectName("ObjectDock");
	m_objectDock->setWidget(m_manager->objectView());
	addDockWidget(Qt::RightDockWidgetArea, m_propertyDock);
	m_propertyDock->setObjectName("PropertyDock");
	m_propertyDock->setWidget(m_propertyView);
	m_propertyView->setModel(m_manager->propertyModel());
	//late GUI settings
	setupGUI(QSize(700, 500));
	setCaption(i18nc("The application's name", "Paladesign"));
	statusBar()->hide();
}

Paladesign::MainWindow::~MainWindow()
{
	delete m_removeRelationAct;
	delete m_addRelationAct;
	delete m_selectShapeAct;
	delete m_toggleObjectDockAct;
	delete m_togglePropertyDockAct;
	delete m_objectDock;
	delete m_propertyDock;
	delete m_propertyView;
}

void Paladesign::MainWindow::selectShape()
{
	//ask user for name of SVG file
	QString target = KFileDialog::getOpenFileName(KUrl("kfiledialog:///paladesign"), "*.svg *.svgz|" + i18nc("Used as filter description in a file dialog.", "Scalabale vector graphics(*.svg, *.svgz)"), m_manager->window(), i18nc("Used as caption for file dialog.", "Select shape - Paladesign"));
	if (target.isEmpty()) //process aborted by user
		return;
	//download file if necessary
	KUrl url(target); QString localFile;
	if (url.isLocalFile())
	{
		if (!KIO::NetAccess::download(url, localFile, 0))
		{
			KMessageBox::error(0, KIO::NetAccess::lastErrorString());
			return;
		}
	}
	else
		localFile = url.path();
	//load shape
	m_manager->shapes()->setShape(localFile);
	m_manager->view()->update();
	KIO::NetAccess::removeTempFile(localFile);
}

#include "mainwindow.moc"

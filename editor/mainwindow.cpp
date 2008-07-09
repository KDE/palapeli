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
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardAction>
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
	//file actions
	KStandardAction::openNew(this, SLOT(newPattern()), actionCollection());
	KStandardAction::open(this, SLOT(loadPattern()), actionCollection());
	KStandardAction::save(this, SLOT(savePattern()), actionCollection());
	KStandardAction::saveAs(this, SLOT(savePatternAs()), actionCollection());
	//docker actions
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
	setCentralWidget(m_manager->tabWidget());
	m_manager->tabWidget()->addTab(m_manager->view(), KIcon(""), i18n("Piece arrangement"));
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
	KUrl target = KFileDialog::getOpenUrl(KUrl("kfiledialog:///paladesign-shape"), "*.svg *.svgz|" + i18nc("Used as filter description in a file dialog.", "Scalable vector graphics (*.svg, *.svgz)"), m_manager->window(), i18nc("Used as caption for file dialog.", "Select shape - Paladesign"));
	if (target.isEmpty()) //process aborted by user
		return;
	//load shape
	m_manager->shapes()->setShape(target);
	m_manager->view()->update();
}

void Paladesign::MainWindow::newPattern()
{
	if (m_manager->isPatternChanged())
	{
		int returnValue = KMessageBox::warningContinueCancel(m_manager->window(), i18n("You have changed the current pattern without saving. Do you want to discard these changes?"));
		if (returnValue != KMessageBox::Continue)
			return;
	}
	m_manager->newPattern();
}

void Paladesign::MainWindow::loadPattern()
{
	if (m_manager->isPatternChanged())
	{
		int returnValue = KMessageBox::warningContinueCancel(m_manager->window(), i18n("You have changed the current pattern without saving. Do you want to discard these changes?"));
		if (returnValue != KMessageBox::Continue)
			return;
	}
	//ask user for name of SVG file
	KUrl target = KFileDialog::getOpenUrl(KUrl("kfiledialog:///paladesign-pattern"), "*.pprp|" + i18nc("Used as filter description in a file dialog.", "Palapeli Regular Patterns (*.pprp)"), m_manager->window(), i18nc("Used as caption for file dialog.", "Load pattern - Paladesign"));
	if (target.isEmpty()) //process aborted by user
		return;
	//load pattern; remember target for future calls to Paladesign::MainWindow::savePattern
	m_manager->loadPattern(target);
	m_saveTarget = target;
}

void Paladesign::MainWindow::savePattern()
{
	if (m_saveTarget.isEmpty())
		savePatternAs();
	else
		m_manager->savePattern(m_saveTarget);
}

void Paladesign::MainWindow::savePatternAs()
{
	//ask user for name of SVG file
	KUrl target = KFileDialog::getSaveUrl(KUrl("kfiledialog:///paladesign-pattern"),"*.pprp|" + i18nc("Used as filter description in a file dialog.", "Palapeli Regular Patterns (*.pprp)"), m_manager->window(), i18nc("Used as caption for file dialog.", "Save pattern - Paladesign"));
	if (target.isEmpty()) //process aborted by user
		return;
	//save pattern; remember target for future calls to Paladesign::MainWindow::savePattern
	m_saveTarget = target;
	m_manager->savePattern(m_saveTarget);
}

#include "mainwindow.moc"

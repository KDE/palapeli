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

#include "manager.h"
#include "mainwindow.h"
#include "objview.h"
#include "points.h"
#include "propmodel.h"
#include "shapes.h"
#include "relation.h"
#include "view.h"

#include <KLocalizedString>

Paladesign::Manager::Manager()
	: m_points(new Paladesign::Points(this))
	, m_shapes(new Paladesign::Shapes)
	, m_propModel(new Paladesign::PropertyModel)
	, m_objView(new Paladesign::ObjectView)
	, m_view(new Paladesign::View(this))
	, m_window(new Paladesign::MainWindow(this))
{
	//main window is deleted by Paladesign::Manager::~Manager because there are widely-spread references to m_view, and m_view will be deleted by m_window
	m_window->setAttribute(Qt::WA_DeleteOnClose, false);
	//physical relations
	Paladesign::Relation* relation = new Paladesign::PhysicalRelation(1.0, 0, 0);
	m_relations << relation;
	QObject::connect(relation, SIGNAL(interactorChanged()), m_view, SLOT(update()));
	QObject::connect(relation, SIGNAL(mouseStateChanged()), m_view, SLOT(update()));
	relation = new Paladesign::PhysicalRelation(1.0, 90, 90);
	m_relations << relation;
	QObject::connect(relation, SIGNAL(interactorChanged()), m_view, SLOT(update()));
	QObject::connect(relation, SIGNAL(mouseStateChanged()), m_view, SLOT(update()));
	//object view
	static const QString physicalRelationCaption = i18n("Physical relation %1");
	m_objView->addObject(m_relations[0], physicalRelationCaption.arg(1));
	m_objView->addObject(m_relations[1], physicalRelationCaption.arg(2));
	QObject::connect(m_objView, SIGNAL(selected(QObject*)), m_view, SLOT(select(QObject*)));
	//property model
	m_propModel->setShowInheritedProperties(false);
	m_propModel->addDisplayString("angleStep", i18nc("an angle in degrees (represented by the ° symbol as in 180°)", "%1°"));
	m_propModel->addLocalizedCaption("angleStep", i18n("Angle difference"));
	m_propModel->addDisplayString("distance", i18n("%1 units"));
	m_propModel->addLocalizedCaption("distance", i18n("Piece distance"));
	m_propModel->addDisplayString("angle", i18nc("an angle in degrees (represented by the ° symbol as in 180°)", "%1°"));
	m_propModel->addLocalizedCaption("angle", i18n("Axis pitch"));
	m_propModel->addDisplayString("relation1Step", i18n("%1 pieces"));
	m_propModel->addLocalizedCaption("relation1Step", i18nc("PR = Physical relation, please translate the acronym", "Steps along PR 1"));
	m_propModel->addDisplayString("relation2Step", i18n("%1 pieces"));
	m_propModel->addLocalizedCaption("relation2Step", i18nc("PR = Physical relation, please translate the acronym", "Steps along PR 2"));
}

Paladesign::Manager::~Manager()
{
	delete m_objView;
	delete m_propModel;
	while (!m_relations.isEmpty())
		delete m_relations.takeFirst();
	delete m_shapes;
	delete m_points;
	//delete m_window; -- causes a crash (not so important anyway because we're exiting so nobody will notice the leak)
}

Paladesign::Points* Paladesign::Manager::points() const
{
	return m_points;
}

Paladesign::Shapes* Paladesign::Manager::shapes() const
{
	return m_shapes;
}

Paladesign::PropertyModel* Paladesign::Manager::propertyModel() const
{
	return m_propModel;
}

Paladesign::ObjectView* Paladesign::Manager::objectView() const
{
	return m_objView;
}

Paladesign::View* Paladesign::Manager::view() const
{
	return m_view;
}

Paladesign::MainWindow* Paladesign::Manager::window() const
{
	return m_window;
}

Paladesign::Relation* Paladesign::Manager::relation(int index) const
{
	return m_relations.value(index, 0);
}

int Paladesign::Manager::relationCount() const
{
	return m_relations.count();
}

bool Paladesign::Manager::addRelation(Paladesign::LogicalRelation* relation)
{
	static int relationIndex = 0;
	static const QString relationCaption = i18n("Logical relation %1");
	if (relation == 0)
		return false;
	//add relation
	m_relations << relation;
	QObject::connect(relation, SIGNAL(interactorChanged()), m_view, SLOT(update()));
	QObject::connect(relation, SIGNAL(mouseStateChanged()), m_view, SLOT(update()));
	//connect to views
	++relationIndex;
	m_objView->addObject(relation, relationCaption.arg(relationIndex));
	m_view->update();
	return true;
}

bool Paladesign::Manager::removeRelation(int index)
{
	if (index < 2 || index >= m_relations.count()) //do not delete non-existent or physical relations
		return false;
	Paladesign::Relation* relation = m_relations.takeAt(index);
	//disconnect from views
	m_objView->removeObject(relation);
	delete relation;
	m_view->update();
	return true;
}

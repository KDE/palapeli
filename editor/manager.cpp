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
#include "../storage/gamestorage.h"
#include "../storage/gamestorageattribs.h"
#include "../storage/gamestorageitem.h"

#include <QTabWidget>
#include <KConfig>
#include <KConfigGroup>
#include <KIO/NetAccess>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardDirs>
#include <KUrl>

namespace Paladesign
{

	namespace Strings
	{
		//strings in .pprpc files (pprpc = PalaPeli Regular Pattern Configuration)
		const QString GeneralGroupKey("Pattern");
		const QString ShapeCountKey("ShapeCount");
		const QString RelationCountKey("RelationCount");
		const QString ShapeGroupKey("Shapes");
		const QString ShapeIdKey("ShapeId-%1");
		const QString RelationGroupKey("Relations");
		const QString PhysicalDistanceKey("PieceDistance-%1");
		const QString PhysicalAxisPitchKey("AxisPitch-%1");
		const QString PhysicalAngleDifferenceKey("AngleDifference-%1");
		const QString LogicalStepKey("PieceStep-%1");
	};

}

Paladesign::Manager::Manager()
	: m_points(new Paladesign::Points(this))
	, m_shapes(0) //will be initialized in newPattern()
	, m_propModel(new Paladesign::PropertyModel)
	, m_objView(new Paladesign::ObjectView)
	, m_tabWidget(new QTabWidget)
	, m_view(new Paladesign::View(this))
	, m_window(new Paladesign::MainWindow(this))
	, m_patternChanged(false)
{
	//main window is deleted by Paladesign::Manager::~Manager because there are widely-spread references to m_view, and m_view will be deleted by m_window
	m_window->setAttribute(Qt::WA_DeleteOnClose, false);
	//cleanup of now unused items (will mostly be shapes)
	Palapeli::GameStorage gs;
	Palapeli::GameStorageItems items = gs.queryItems(Palapeli::GameStorageAttributes() << new Palapeli::GameStorageNoDependencyAttribute);
	foreach (const Palapeli::GameStorageItem& item, items)
		gs.removeItem(item);
	//initialize editor
	newPattern();
	QObject::connect(m_objView, SIGNAL(selected(QObject*)), m_view, SLOT(select(QObject*)));
	//initialize property model
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
	foreach (QString tempFile, m_tempFiles)
		KIO::NetAccess::removeTempFile(tempFile);
	delete m_objView;
	delete m_propModel;
	while (!m_relations.isEmpty())
		delete m_relations.takeFirst();
	delete m_shapes;
	delete m_points;
	delete m_view;
	//delete m_window; -- causes a crash (not so important anyway because we're exiting so nobody will notice the leak)
}

bool Paladesign::Manager::isPatternChanged() const
{
	return m_patternChanged;
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

QTabWidget* Paladesign::Manager::tabWidget() const
{
	return m_tabWidget;
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
	connect(relation, SIGNAL(interactorChanged()), m_view, SLOT(update()));
	connect(relation, SIGNAL(mouseStateChanged()), m_view, SLOT(update()));
	connect(relation, SIGNAL(interactorChanged()), this, SLOT(patternChanged()));
	//connect to views
	++relationIndex;
	m_objView->addObject(relation, relationCaption.arg(relationIndex));
	m_view->update();
	//adding a relation is a change
	m_patternChanged = true;
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
	//removing a relation is a change
	m_patternChanged = true;
	return true;
}

void Paladesign::Manager::patternChanged()
{
	m_patternChanged = true;
}

void Paladesign::Manager::newPattern()
{
	//clear state
	m_propModel->setObject(0);
	m_objView->clear();
	while (!m_relations.isEmpty())
		delete m_relations.takeFirst();
	delete m_shapes;
	m_patternChanged = false;
	//shapes
	m_shapes = new Paladesign::Shapes();
	connect(m_shapes, SIGNAL(shapeChanged()), this, SLOT(patternChanged()));
	//physical relations
	Paladesign::Relation* relation = new Paladesign::PhysicalRelation(1.0, 0, 0);
	m_relations << relation;
	connect(relation, SIGNAL(interactorChanged()), m_view, SLOT(update()));
	connect(relation, SIGNAL(mouseStateChanged()), m_view, SLOT(update()));
	connect(relation, SIGNAL(interactorChanged()), this, SLOT(patternChanged()));
	relation = new Paladesign::PhysicalRelation(1.0, 90, 90);
	m_relations << relation;
	connect(relation, SIGNAL(interactorChanged()), m_view, SLOT(update()));
	connect(relation, SIGNAL(mouseStateChanged()), m_view, SLOT(update()));
	connect(relation, SIGNAL(interactorChanged()), this, SLOT(patternChanged()));
	//object view
	static const QString physicalRelationCaption = i18n("Physical relation %1");
	m_objView->addObject(m_relations[0], physicalRelationCaption.arg(1));
	m_objView->addObject(m_relations[1], physicalRelationCaption.arg(2));
	//issue graphics update
	m_view->update();
}

void Paladesign::Manager::loadPattern(const KUrl& url)
{
	//import items from pattern into storage
	Palapeli::GameStorage gs;
	Palapeli::GameStorageItems items = gs.importItems(url);
	//find pattern configuration
	Palapeli::GameStorageAttributes configurationAttributes;
	configurationAttributes << new Palapeli::GameStorageExtensionAttribute(".pprpc");
	Palapeli::GameStorageItems configurationItems = gs.filterItems(items, configurationAttributes);
	if (configurationItems.count() == 0)
	{
		//no configuration found - remove all imported items and do nothing more
		foreach (const Palapeli::GameStorageItem& item, items)
			gs.removeItem(item);
		return;
	}
	Palapeli::GameStorageItem configurationItem = configurationItems[0];
	m_configurationId = configurationItem.id();
	//load configuration
	KConfig configuration(configurationItem.filePath());
	//reset editor
	newPattern();
	//general information
	KConfigGroup generalGroup(&configuration, Paladesign::Strings::GeneralGroupKey);
	const int shapeCount = 1; //TODO: shapeCount = generalGroup.readEntry(Paladesign::Strings::ShapeCountKey, 0);
	const int relationCount = generalGroup.readEntry(Paladesign::Strings::RelationCountKey, 2);
	//shape information
	KConfigGroup shapeGroup(&configuration, Paladesign::Strings::ShapeGroupKey);
	for (int i = 0; i < shapeCount; ++i)
		m_shapes->setShape(QUuid(shapeGroup.readEntry(Paladesign::Strings::ShapeIdKey.arg(i + 1), QString())));
	//physical relation information
	KConfigGroup relationGroup(&configuration, Paladesign::Strings::RelationGroupKey);
	Paladesign::PhysicalRelation* relation = qobject_cast<Paladesign::PhysicalRelation*>(m_relations[0]);
	relation->setDistance(relationGroup.readEntry(Paladesign::Strings::PhysicalDistanceKey.arg(1), 0.0));
	relation->setAngle(relationGroup.readEntry(Paladesign::Strings::PhysicalAxisPitchKey.arg(1), 0));
	relation->setAngleStep(relationGroup.readEntry(Paladesign::Strings::PhysicalAngleDifferenceKey.arg(1), 0));
	relation = qobject_cast<Paladesign::PhysicalRelation*>(m_relations[1]);
	relation->setDistance(relationGroup.readEntry(Paladesign::Strings::PhysicalDistanceKey.arg(2), 0.0));
	relation->setAngle(relationGroup.readEntry(Paladesign::Strings::PhysicalAxisPitchKey.arg(2), 0));
	relation->setAngleStep(relationGroup.readEntry(Paladesign::Strings::PhysicalAngleDifferenceKey.arg(2), 0));
	for (int i = 2; i < relationCount; ++i)
	{
		QPoint relationStep = relationGroup.readEntry(Paladesign::Strings::LogicalStepKey.arg(i - 2), QPoint());
		Paladesign::LogicalRelation* relation = new Paladesign::LogicalRelation(relationStep.x(), relationStep.y(), this);
		addRelation(relation);
	}
}

void Paladesign::Manager::savePattern(const KUrl& url)
{
	//create configuration file if not available
	Palapeli::GameStorage gs;
	Palapeli::GameStorageItem configurationItem;
	if (m_configurationId.isNull())
	{
		//create a .pprpc file (= PalaPeli Regular Pattern Configuration)
		configurationItem = gs.addItem(".pprpc", Palapeli::GameStorageItem::GlobalResource);
		m_configurationId = configurationItem.id();
	}
	else
		configurationItem = gs.item(m_configurationId);
	//find shape file
	Palapeli::GameStorageItem shapeItem = gs.item(m_shapes->shapeId());
	//open configuration file
	KConfig configuration(configurationItem.filePath());
	//general information
	KConfigGroup generalGroup(&configuration, Paladesign::Strings::GeneralGroupKey);
	generalGroup.writeEntry(Paladesign::Strings::ShapeCountKey, 1);
	generalGroup.writeEntry(Paladesign::Strings::RelationCountKey, m_relations.count());
	//shape information
	KConfigGroup shapeGroup(&configuration, Paladesign::Strings::ShapeGroupKey);
	shapeGroup.writeEntry(Paladesign::Strings::ShapeIdKey.arg(1), m_shapes->shapeId().toString());
	//physical relation information
	KConfigGroup relationGroup(&configuration, Paladesign::Strings::RelationGroupKey);
	Paladesign::PhysicalRelation* relation = qobject_cast<Paladesign::PhysicalRelation*>(m_relations[0]);
	relationGroup.writeEntry(Paladesign::Strings::PhysicalDistanceKey.arg(1), relation->distance());
	relationGroup.writeEntry(Paladesign::Strings::PhysicalAxisPitchKey.arg(1), relation->angle());
	relationGroup.writeEntry(Paladesign::Strings::PhysicalAngleDifferenceKey.arg(1), relation->angleStep());
	relation = qobject_cast<Paladesign::PhysicalRelation*>(m_relations[1]);
	relationGroup.writeEntry(Paladesign::Strings::PhysicalDistanceKey.arg(2), relation->distance());
	relationGroup.writeEntry(Paladesign::Strings::PhysicalAxisPitchKey.arg(2), relation->angle());
	relationGroup.writeEntry(Paladesign::Strings::PhysicalAngleDifferenceKey.arg(2), relation->angleStep());
	for (int i = 2; i < m_relations.count(); ++i)
	{
		Paladesign::LogicalRelation* relation = qobject_cast<Paladesign::LogicalRelation*>(m_relations[i]);
		relationGroup.writeEntry(Paladesign::Strings::LogicalStepKey.arg(i - 2), QPoint(relation->relation1Step(), relation->relation2Step()));
	}
	//finish configuration
	configuration.sync();
	//export affected items
	Palapeli::GameStorageItems items;
	items << configurationItem << shapeItem;
	gs.exportItems(url, items);
}

#include "manager.moc"

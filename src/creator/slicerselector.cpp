/*
    SPDX-FileCopyrightText: 2009, 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "slicerselector.h"

#include <Pala/Slicer>
#include <Pala/SlicerJob>
#include <Pala/SlicerMode>

#include <KPluginFactory>
#include <KPluginMetaData>
#include <KPluginLoader>

Palapeli::SlicerSelector::SlicerSelector(QWidget* parent)
	: QTreeWidget(parent)
{
	qRegisterMetaType<Palapeli::SlicerSelection>();
	setHeaderHidden(true);
	setSelectionBehavior(QAbstractItemView::SelectItems);
	setSelectionMode(QAbstractItemView::SingleSelection);
	connect(this, &SlicerSelector::itemSelectionChanged, this, &SlicerSelector::slotSelectionChanged);
	//load slicer plugins
	const QVector<KPluginMetaData> offers = KPluginLoader::findPlugins(QStringLiteral("palapelislicers"));
	for (const KPluginMetaData &offer : offers)
	{
		const QString pluginName = offer.pluginId(), slicerName = offer.name();
		//create slicer object
		KPluginLoader loader(offer.fileName());
		KPluginFactory *factory = loader.factory();
		Pala::Slicer* slicer = factory->create<Pala::Slicer>(nullptr, QVariantList());
		if (!slicer)
			continue;
		m_slicerInstances << slicer;
		//create item for this slicer
		QTreeWidgetItem* slicerItem = new QTreeWidgetItem(this);
		slicerItem->setData(0, Qt::DisplayRole, slicerName);
		//scan mode list
		const QList<const Pala::SlicerMode*> modes = slicer->modes();
		if (modes.isEmpty())
		{
			//no modes - make slicer item selectable (i.e. fallback to list-like behavior)
			slicerItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			Palapeli::SlicerSelection sel(pluginName, slicer);
			m_knownSelections << sel;
			//the index in m_knownSelections is recorded in Qt::UserRole to map selected items to SlicerSelections
			slicerItem->setData(0, Qt::UserRole, m_knownSelections.count() - 1);
		}
		else
		{
			//slicer has modes - require to select a specific mode
			slicerItem->setFlags(Qt::ItemIsEnabled);
			for (const Pala::SlicerMode* mode : modes)
			{
				QTreeWidgetItem* modeItem = new QTreeWidgetItem(slicerItem);
				modeItem->setData(0, Qt::DisplayRole, mode->name());
				Palapeli::SlicerSelection sel(pluginName, slicer, mode);
				m_knownSelections << sel;
				//the index in m_knownSelections is recorded in Qt::UserRole to map selected items to SlicerSelections
				modeItem->setData(0, Qt::UserRole, m_knownSelections.count() - 1);
			}
		}
	}
	//always show everything
	setItemsExpandable(false);
	expandAll();
}

Palapeli::SlicerSelector::~SlicerSelector()
{
	qDeleteAll(m_slicerInstances);
}

QList<const Pala::Slicer*> Palapeli::SlicerSelector::slicers() const
{
	QList<const Pala::Slicer*> result;
	for (Pala::Slicer* slicer : m_slicerInstances)
		result << slicer;
	return result;
}

Palapeli::SlicerSelection Palapeli::SlicerSelector::currentSelection() const
{
	QTreeWidgetItem* item = selectedItems().value(0);
	if (item)
	{
		const QVariant selIndex = item->data(0, Qt::UserRole);
		if (selIndex.type() == QVariant::Int)
			return m_knownSelections.value(selIndex.toInt());
	}
	//empty or invalid selection
	return Palapeli::SlicerSelection();
}

void Palapeli::SlicerSelector::slotSelectionChanged()
{
	Q_EMIT currentSelectionChanged(currentSelection());
}



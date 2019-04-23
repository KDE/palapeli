/***************************************************************************
 *   Copyright 2009, 2010 Stefan Majewsky <majewsky@gmx.net>
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

#include "slicerselector.h"
#include "../libpala/slicer.h"
#include "../libpala/slicerjob.h"
#include "../libpala/slicermode.h"

#include <KServiceTypeTrader>

Palapeli::SlicerSelector::SlicerSelector(QWidget* parent)
	: QTreeWidget(parent)
{
	qRegisterMetaType<Palapeli::SlicerSelection>();
	setHeaderHidden(true);
	setSelectionBehavior(QAbstractItemView::SelectItems);
	setSelectionMode(QAbstractItemView::SingleSelection);
	connect(this, &SlicerSelector::itemSelectionChanged, this, &SlicerSelector::slotSelectionChanged);
	//load slicer plugins
	KService::List offers = KServiceTypeTrader::self()->query(QStringLiteral("Libpala/SlicerPlugin"));
	foreach (KService::Ptr offer, offers)
	{
		const QString pluginName = offer->library(), slicerName = offer->name();
		//create slicer object
		Pala::Slicer* slicer = offer->createInstance<Pala::Slicer>(0, QVariantList());
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
			foreach (const Pala::SlicerMode* mode, modes)
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
	foreach (Pala::Slicer* slicer, m_slicerInstances)
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
	emit currentSelectionChanged(currentSelection());
}



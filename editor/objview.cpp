/***************************************************************************
 *   Copyright 2008 Stefan Majewsky <majewsky@gmx.net>
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

#include "objview.h"

Paladesign::ObjectView::ObjectView()
	: QListWidget()
{
	connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(selectedItemChanged()));
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setSelectionMode(QAbstractItemView::SingleSelection);
}

void Paladesign::ObjectView::addObject(QObject* object, const QString& caption)
{
	QListWidgetItem* item = new QListWidgetItem(caption, this);
	item->setData(ObjectPointerRole, QVariant::fromValue<QObject*>(object));
}

void Paladesign::ObjectView::removeObject(QObject* object)
{
	for (int i = 0; i < count(); ++i)
	{
		QListWidgetItem* currentItem = item(i);
		QObject* pointer = currentItem->data(ObjectPointerRole).value<QObject*>();
		if (pointer == object)
		{
			removeItemWidget(currentItem);
			delete currentItem;
		}
	}
}

void Paladesign::ObjectView::selectedItemChanged()
{
	QList<QListWidgetItem*> selection = selectedItems();
	if (selection.count() == 0)
		emit selected(0);
	else
	{
		QObject* pointer = selection.at(0)->data(ObjectPointerRole).value<QObject*>();
		emit selected(pointer);
	}
}

void Paladesign::ObjectView::changeSelectedItem(QObject* object)
{
	bool selectedAnItem = false;
	//search for item associated with this object and select it
	for (int i = 0; i < count(); ++i)
	{
		QListWidgetItem* currentItem = item(i);
		QObject* pointer = currentItem->data(ObjectPointerRole).value<QObject*>();
		if (pointer == object)
		{
			selectedAnItem = true;
			if (!currentItem->isSelected())
			{
				setCurrentItem(currentItem);
				emit selected(pointer);
			}
			break;
		}
	}
	//no associated item found (i.e. object == 0 or not in this list)
	if (!selectedAnItem)
	{
		foreach (QListWidgetItem* selectedItem, selectedItems())
			selectedItem->setSelected(false);
		emit selected(0);
	}
}

#include "objview.moc"

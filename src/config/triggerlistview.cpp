/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
***************************************************************************/

#include "triggerlistview.h"
#include "triggerlistview_p.h"

static QString categoryToString(Palapeli::Interactor::Category category)
{
	switch (category)
	{
		case Palapeli::Interactor::PieceInteraction:
			return i18n("Interaction with pieces");
		case Palapeli::Interactor::TableInteraction:
			return i18n("Interaction with the puzzle table");
		case Palapeli::Interactor::ViewportInteraction:
			return i18n("Interaction with the viewport");
		case Palapeli::Interactor::NoCategory: default:
			return QString();
	}
}

Palapeli::TriggerListView::TriggerListView(const QMap<QByteArray, Palapeli::Interactor*>& interactors, Palapeli::InteractorType interactorType, QWidget* parent)
	: KCategorizedView(parent)
	, m_categoryDrawer(new KCategoryDrawer(this))
	, m_baseModel(new QStandardItemModel(this))
	, m_proxyModel(new Palapeli::TriggerListProxyModel(this))
	, m_delegate(new Palapeli::TriggerListDelegate(this))
{
	//fill base model with interactors
	QMap<QByteArray, Palapeli::Interactor*>::const_iterator it1 = interactors.begin(), it2 = interactors.end();
	for (; it1 != it2; ++it1)
	{
		//filter interactor
		Palapeli::Interactor* interactor = it1.value();
		if (interactor->interactorType() != interactorType)
			continue;
		//create item for interactor
		QStandardItem* item = new QStandardItem;
		item->setData(interactor->description(), Qt::DisplayRole);
		item->setData(interactor->icon(), Qt::DecorationRole);
		item->setData(interactorType, Palapeli::InteractorTypeRole);
		item->setData(it1.key(), Palapeli::InteractorRole);
		item->setData(qVariantFromValue(Palapeli::Trigger()), Palapeli::TriggerRole);
		item->setData(categoryToString(interactor->category()), KCategorizedSortFilterProxyModel::CategoryDisplayRole);
		item->setData(interactor->category(), KCategorizedSortFilterProxyModel::CategorySortRole);
		m_baseModel->appendRow(item);
	}
	//setup model/view
	m_proxyModel->setSourceModel(m_baseModel);
	setModel(m_proxyModel);
	setItemDelegate(m_delegate);
	connect(m_delegate, SIGNAL(triggerChanged()), SIGNAL(associationsChanged()));
// 	setCategoryDrawer(m_categoryDrawer); //FIXME: Why do I crash?
}

Palapeli::TriggerListView::~TriggerListView()
{
	delete m_categoryDrawer;
}

void Palapeli::TriggerListView::getAssociations(QMap<QByteArray, Palapeli::Trigger>& associations)
{
	for (int i = 0; i < m_baseModel->rowCount(); ++i)
	{
		QStandardItem* item = m_baseModel->item(i);
		const QByteArray interactor = item->data(Palapeli::InteractorRole).value<QByteArray>();
		const Palapeli::Trigger trigger = item->data(Palapeli::TriggerRole).value<Palapeli::Trigger>();
		if (trigger.isValid())
			associations.insertMulti(interactor, trigger);
	}
}

void Palapeli::TriggerListView::setAssociations(const QMap<QByteArray, Palapeli::Trigger>& associations)
{
	for (int i = 0; i < m_baseModel->rowCount(); ++i)
	{
		QStandardItem* item = m_baseModel->item(i);
		const QByteArray interactor = item->data(Palapeli::InteractorRole).value<QByteArray>();
		item->setData(qVariantFromValue(associations.value(interactor)), Palapeli::TriggerRole);
	}
}


//

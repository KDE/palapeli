/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "triggerlistview.h"
#include "triggerlistview_p.h"
#include <KCategoryDrawer>
#include <KLocalizedString>
#include <QStandardItemModel>
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
		item->setData(QVariant::fromValue(Palapeli::Trigger()), Palapeli::TriggerRole);
		item->setData(categoryToString(interactor->category()), KCategorizedSortFilterProxyModel::CategoryDisplayRole);
		item->setData(interactor->category(), KCategorizedSortFilterProxyModel::CategorySortRole);
		m_baseModel->appendRow(item);
	}
	//setup model/view
	m_proxyModel->setSourceModel(m_baseModel);
	setModel(m_proxyModel);
	setItemDelegate(m_delegate);
	connect(m_delegate, &Palapeli::TriggerListDelegate::triggerChanged, this, &TriggerListView::associationsChanged);
// 	setCategoryDrawer(m_categoryDrawer); //FIXME: Why do I crash?
}

Palapeli::TriggerListView::~TriggerListView()
{
	delete m_categoryDrawer;
	// Work-around for bug 487499
	// Delete the delegate before the itemview deletes the viewport
	// and that one its children, among them the widgets the delegate
	// has returned in createItemWidgets.
	// Otherwise the delegate will wrongly warn about custom deletion
	// of those widgets.
	delete m_delegate;
}

void Palapeli::TriggerListView::getAssociations(QMap<QByteArray, Palapeli::Trigger>& associations)
{
	for (int i = 0; i < m_baseModel->rowCount(); ++i)
	{
		QStandardItem* item = m_baseModel->item(i);
		const QByteArray interactor = item->data(Palapeli::InteractorRole).value<QByteArray>();
		const Palapeli::Trigger trigger = item->data(Palapeli::TriggerRole).value<Palapeli::Trigger>();
		if (trigger.isValid())
			associations.insert(interactor, trigger);
	}
}

void Palapeli::TriggerListView::setAssociations(const QMap<QByteArray, Palapeli::Trigger>& associations)
{
	for (int i = 0; i < m_baseModel->rowCount(); ++i)
	{
		QStandardItem* item = m_baseModel->item(i);
		const QByteArray interactor = item->data(Palapeli::InteractorRole).value<QByteArray>();
		item->setData(QVariant::fromValue(associations.value(interactor)), Palapeli::TriggerRole);
	}
}


//

#include "moc_triggerlistview.cpp"
#include "moc_triggerlistview_p.cpp"

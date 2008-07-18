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

#include "listmenu.h"
#include "listmenu_p.h"

#include <QString>
#include <KIcon>

Palapeli::ListMenuPrivate::ListMenuPrivate(Palapeli::ListMenu* menu)
	: m_model(0)
	, m_mapper(new QSignalMapper(menu))
	, m_menu(menu)
	, m_disableWhenEmpty(false)
	, m_enabled(true)
{
	connect(m_mapper, SIGNAL(mapped(const QString&)), menu, SIGNAL(clicked(const QString&)));
}

Palapeli::ListMenuPrivate::~ListMenuPrivate()
{
	//view was already reset at this point
	delete m_mapper;
}

void Palapeli::ListMenuPrivate::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
	if (m_model == 0 || topLeft.parent().isValid() || bottomRight.parent().isValid())
		return;
	int start = topLeft.row(), end = bottomRight.row();
	for (int i = start; i <= end; ++i)
	{
		QString newText = m_model->data(m_items.at(i)->index, Qt::DisplayRole).toString();
		m_items.at(i)->action->setText(newText);
		m_mapper->setMapping(m_items.at(i)->action, newText);
	}
}

void Palapeli::ListMenuPrivate::modelAboutToBeReset()
{
	if (m_model == 0)
		return;
	//flush menu
	while (m_items.isEmpty())
	{
		Palapeli::ListMenuPrivateItem* item = m_items.takeFirst();
		m_menu->removeAction(item->action);
		m_mapper->removeMappings(item->action);
		delete item->action;
		delete item;
	}
	if (m_disableWhenEmpty)
		m_menu->setEnabled(false);
}

void Palapeli::ListMenuPrivate::modelReset()
{
	if (m_model == 0)
		return;
	//get all new items
	int rowCount = m_model->rowCount();
	for (int i = 0; i < rowCount; ++i)
	{
		Palapeli::ListMenuPrivateItem* item = new Palapeli::ListMenuPrivateItem(m_model->index(i, 0));
		QString actionText = m_model->data(item->index, Qt::DisplayRole).toString();
		item->action = new KAction(actionText, m_menu);
		connect(item->action, SIGNAL(triggered(bool)), m_mapper, SLOT(map()));
		m_mapper->setMapping(item->action, actionText);
		m_menu->addAction(item->action);
		m_items << item;
	}
	m_menu->setEnabled(m_menu->listMenuEnabled());
}

void Palapeli::ListMenuPrivate::rowsInserted(const QModelIndex& parent, int start, int end)
{
	if (m_model == 0 || parent.isValid())
		return;
	//navigate list iterator
	QMutableListIterator<Palapeli::ListMenuPrivateItem*> iterItems(m_items);
	for (int i = 0; i < start && iterItems.hasNext(); ++i)
		iterItems.next();
	//insert new items
	for (int i = start; i <= end; ++i)
	{
		//create item
		Palapeli::ListMenuPrivateItem* item = new Palapeli::ListMenuPrivateItem(m_model->index(i, 0));
		QString actionText = m_model->data(item->index, Qt::DisplayRole).toString();
		item->action = new KAction(actionText, m_menu);
		connect(item->action, SIGNAL(triggered(bool)), m_mapper, SLOT(map()));
		m_mapper->setMapping(item->action, actionText);
		//insert item
		if (iterItems.hasNext())
			m_menu->insertAction(iterItems.peekNext()->action, item->action);
		else
			m_menu->addAction(item->action);
		iterItems.insert(item);
	}
	m_menu->setEnabled(m_menu->listMenuEnabled());
}

void Palapeli::ListMenuPrivate::rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
	if (m_model == 0 || parent.isValid())
		return;
	int removeCount = end - start + 1;
	for (int i = 0; i < removeCount; ++i)
	{
		Palapeli::ListMenuPrivateItem* item = m_items.takeAt(start);
		m_menu->removeAction(item->action);
		m_mapper->removeMappings(item->action);
		delete item->action;
		delete item;
	}
	m_menu->setEnabled(m_menu->listMenuEnabled());
}

Palapeli::ListMenu::ListMenu(const KIcon& icon, const QString& text, QObject* parent)
	: KActionMenu(icon, text, parent)
	, p(new Palapeli::ListMenuPrivate(this))
{
}

Palapeli::ListMenu::ListMenu(const QString& text, QObject* parent)
	: KActionMenu(text, parent)
	, p(new Palapeli::ListMenuPrivate(this))
{
}

Palapeli::ListMenu::ListMenu(QObject* parent)
	: KActionMenu(parent)
	, p(new Palapeli::ListMenuPrivate(this))
{
}

Palapeli::ListMenu::~ListMenu()
{
	if (p->m_model != 0)
		p->modelReset();
	delete p;
}

bool Palapeli::ListMenu::isDisabledWhenEmpty() const
{
	return p->m_disableWhenEmpty;
}

bool Palapeli::ListMenu::listMenuEnabled() const
{
	return p->m_enabled;
}

QAbstractListModel* Palapeli::ListMenu::model() const
{
	return p->m_model;
}

void Palapeli::ListMenu::setDisabledWhenEmpty(bool disabledWhenEmpty)
{
	p->m_disableWhenEmpty = disabledWhenEmpty;
}

void Palapeli::ListMenu::setEnabled(bool enabled)
{
	p->m_enabled = enabled;
	//apply setting
	if (enabled)
		KActionMenu::setEnabled(p->m_disableWhenEmpty ? p->m_model->rowCount() > 0 : true);
	else
		KActionMenu::setEnabled(false);
}

void Palapeli::ListMenu::setModel(QAbstractListModel* model)
{
	//disconnect from old model
	if (p->m_model != 0)
	{
		disconnect(p->m_model, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), p, SLOT(dataChanged(const QModelIndex&, const QModelIndex&)));
		disconnect(p->m_model, SIGNAL(modelAboutToBeReset()), p, SLOT(modelAboutToBeReset()));
		disconnect(p->m_model, SIGNAL(modelReset()), p, SLOT(modelReset()));
		disconnect(p->m_model, SIGNAL(rowsInserted(const QModelIndex&, int, int)), p, SLOT(rowsInserted(const QModelIndex&, int, int)));
		disconnect(p->m_model, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)), p, SLOT(rowsAboutToBeRemoved(const QModelIndex&, int, int)));
	}
	//switch model
	p->modelAboutToBeReset();
	p->m_model = model;
	p->modelReset();
	//connect to new model
	if (p->m_model != 0)
	{
		connect(p->m_model, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), p, SLOT(dataChanged(const QModelIndex&, const QModelIndex&)));
		connect(p->m_model, SIGNAL(modelAboutToBeReset()), p, SLOT(modelAboutToBeReset()));
		connect(p->m_model, SIGNAL(modelReset()), p, SLOT(modelReset()));
		connect(p->m_model, SIGNAL(rowsInserted(const QModelIndex&, int, int)), p, SLOT(rowsInserted(const QModelIndex&, int, int)));
		connect(p->m_model, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)), p, SLOT(rowsAboutToBeRemoved(const QModelIndex&, int, int)));
	}
}

#include "listmenu.moc"
#include "listmenu_p.moc"

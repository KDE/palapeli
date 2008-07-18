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

#ifndef PALAPELI_LISTMENU_P_H
#define PALAPELI_LISTMENU_P_H

#include <QAbstractListModel>
#include <QList>
#include <QModelIndex>
#include <QObject>
#include <QPersistentModelIndex>
#include <QSignalMapper>
#include <KAction>

namespace Palapeli
{

	struct ListMenuPrivateItem
	{
		explicit ListMenuPrivateItem(const QModelIndex& itemIndex) : index(itemIndex) {}
		explicit ListMenuPrivateItem(const QPersistentModelIndex& itemIndex) : index(itemIndex) {}
		QPersistentModelIndex index;
		KAction* action;
	};

	class ListMenuPrivate : public QObject
	{
		Q_OBJECT
		public:
			QAbstractListModel* m_model;
			QList<ListMenuPrivateItem*> m_items;
			QSignalMapper* m_mapper;
			ListMenu* m_menu;

			bool m_disableWhenEmpty, m_enabled;

			ListMenuPrivate(ListMenu* menu);
			~ListMenuPrivate();
		public Q_SLOTS:
			void dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
			//header data is not implemented
			//TODO: layout changing is not implemented
			void modelAboutToBeReset();
			void modelReset();
			void rowsInserted(const QModelIndex& parent, int start, int end);
			void rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
	};

}

#endif // PALAPELI_LISTMENU_P_H

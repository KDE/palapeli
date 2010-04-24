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

#ifndef PALAPELI_TRIGGERLISTVIEW_H
#define PALAPELI_TRIGGERLISTVIEW_H

#include "../engine/interactor.h"
#include "../engine/trigger.h"

class QStandardItemModel;
class KCategorizedSortFilterProxyModel;
class KCategoryDrawer;
#include <KCategorizedView>

namespace Palapeli
{
	class Interactor;
	class TriggerListDelegate;
	enum TriggerListRoles { TriggerRole = Qt::UserRole + 42, InteractorRole };

	class TriggerListView : public KCategorizedView
	{
		public:
			TriggerListView(const QMap<QByteArray, Palapeli::Interactor*>& interactors, Palapeli::InteractorType interactorType, QWidget* parent = 0);
			virtual ~TriggerListView();

			void getAssociations(QMap<QByteArray, Palapeli::Trigger>& associations);
			void setAssociations(const QMap<QByteArray, Palapeli::Trigger>& associations);
		private:
			KCategoryDrawer* m_categoryDrawer;
			QStandardItemModel* m_baseModel;
			KCategorizedSortFilterProxyModel* m_proxyModel;
			Palapeli::TriggerListDelegate* m_delegate;
	};
}

#endif // PALAPELI_TRIGGERLISTVIEW_H

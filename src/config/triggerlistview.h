/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

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
	enum TriggerListRoles { TriggerRole = Qt::UserRole + 42, InteractorTypeRole, InteractorRole };

	class TriggerListView : public KCategorizedView
	{
		Q_OBJECT
		public:
			TriggerListView(const QMap<QByteArray, Palapeli::Interactor*>& interactors, Palapeli::InteractorType interactorType, QWidget* parent = nullptr);
			~TriggerListView() override;

			void getAssociations(QMap<QByteArray, Palapeli::Trigger>& associations);
			void setAssociations(const QMap<QByteArray, Palapeli::Trigger>& associations);
		Q_SIGNALS:
			void associationsChanged();
		private:
			KCategoryDrawer* m_categoryDrawer;
			QStandardItemModel* m_baseModel;
			KCategorizedSortFilterProxyModel* m_proxyModel;
			Palapeli::TriggerListDelegate* m_delegate;
	};
}

#endif // PALAPELI_TRIGGERLISTVIEW_H

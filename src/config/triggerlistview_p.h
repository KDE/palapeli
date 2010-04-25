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

#ifndef PALAPELI_TRIGGERLISTVIEW_P_H
#define PALAPELI_TRIGGERLISTVIEW_P_H

#include "triggerlistview.h"
#include "elidinglabel.h"
#include "mouseinputbutton.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QStandardItemModel>
#include <KCategorizedSortFilterProxyModel>
#include <KCategoryDrawer>
#include <KLocalizedString>
#include <KStringHandler>
#include <KWidgetItemDelegate>

namespace Palapeli
{
	class TriggerListProxyModel : public KCategorizedSortFilterProxyModel
	{
		public:
			TriggerListProxyModel(QObject* parent = 0)
				: KCategorizedSortFilterProxyModel(parent)
			{
				setCategorizedModel(true);
			}
		protected:
			virtual int compareCategories(const QModelIndex& left, const QModelIndex& right) const
			{
				const int categoryLeft = left.data(KCategorizedSortFilterProxyModel::CategorySortRole).value<int>();
				const int categoryRight = right.data(KCategorizedSortFilterProxyModel::CategorySortRole).value<int>();
				return categoryRight - categoryLeft;
			}
			virtual bool subSortLessThan(const QModelIndex& left, const QModelIndex& right) const
			{
				const QString textLeft = left.data(Qt::DisplayRole).toString();
				const QString textRight = right.data(Qt::DisplayRole).toString();
				return KStringHandler::naturalCompare(textLeft, textRight) < 0;
			}
	};

	class TriggerListDelegateWidget : public QWidget
	{
		Q_OBJECT
		public:
			TriggerListDelegateWidget(QWidget* parent = 0) : QWidget(parent)
			{
				m_iconLabel = new QLabel(this);
				m_nameLabel = new Palapeli::ElidingLabel(this);
				m_inputButton = new Palapeli::MouseInputButton(this);
				connect(m_inputButton, SIGNAL(triggerChanged(const Palapeli::Trigger&)), SIGNAL(triggerChanged(const Palapeli::Trigger&)));
				//construct layout
				QHBoxLayout* layout = new QHBoxLayout;
				setLayout(layout);
				layout->addWidget(m_iconLabel);
				m_iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
				m_iconLabel->setFixedSize(QSize(32, 32));
				layout->addWidget(m_nameLabel);
				layout->addWidget(m_inputButton);
				m_inputButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
			}
			void setIcon(const QIcon& icon)
			{
				//TODO: respect global icon size configuration
				m_iconLabel->setPixmap(icon.pixmap(32));
			}
			void setText(const QString& text)
			{
				m_nameLabel->setFullText(text);
			}
			void setTrigger(const Palapeli::Trigger& trigger)
			{
				m_inputButton->setTrigger(trigger);
			}
			void setInteractorType(Palapeli::InteractorType type)
			{
				m_inputButton->setMouseAllowed(type == Palapeli::MouseInteractor);
				m_inputButton->setWheelAllowed(type == Palapeli::WheelInteractor);
			}
		Q_SIGNALS:
			void triggerChanged(const Palapeli::Trigger& newTrigger);
		private:
			QLabel* m_iconLabel;
			Palapeli::ElidingLabel* m_nameLabel;
			Palapeli::MouseInputButton* m_inputButton;
	};

	class TriggerListDelegate : public KWidgetItemDelegate
	{
		Q_OBJECT
		public:
			explicit TriggerListDelegate(QAbstractItemView* view, QObject* parent = 0) : KWidgetItemDelegate(view, parent)
			{
				m_calculator = new Palapeli::TriggerListDelegateWidget(view);
				m_calculator->setVisible(false);
			}
			virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
			{
				Q_UNUSED(index)
				QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, 0);
			}
			virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
			{
				updateItemWidgets(QList<QWidget*>() << m_calculator, option, index);
				return m_calculator->minimumSizeHint();
			}
		protected:
			virtual QList<QWidget*> createItemWidgets() const
			{
				return QList<QWidget*>() << new Palapeli::TriggerListDelegateWidget(itemView());
			}
			virtual void updateItemWidgets(QList<QWidget*> widgets, const QStyleOptionViewItem& option, const QPersistentModelIndex& index) const
			{
				Palapeli::TriggerListDelegateWidget* widget = qobject_cast<Palapeli::TriggerListDelegateWidget*>(widgets[0]);
				//adjust widget contents
				widget->setIcon(index.data(Qt::DecorationRole).value<QIcon>());
				widget->setText(index.data(Qt::DisplayRole).value<QString>());
				disconnect(widget, 0, this, 0);
				widget->setTrigger(index.data(TriggerRole).value<Palapeli::Trigger>());
				connect(widget, SIGNAL(triggerChanged(const Palapeli::Trigger&)), SLOT(slotTriggerChanged(const Palapeli::Trigger&)));
				//adjust widget geometry
				QRect rect = option.rect;
				rect.moveTop(0);
				widget->setGeometry(rect);
				//adjust widget behavior
				widget->setInteractorType((Palapeli::InteractorType) index.data(Palapeli::InteractorTypeRole).toInt());
			}
		Q_SIGNALS:
			void triggerChanged();
		private Q_SLOTS:
			void slotTriggerChanged(const Palapeli::Trigger& newTrigger)
			{
				const QModelIndex index = focusedIndex();
				QAbstractItemModel* model = const_cast<QAbstractItemModel*>(index.model());
				model->setData(index, qVariantFromValue(newTrigger), Palapeli::TriggerRole);
				emit triggerChanged();
			}
		private:
			Palapeli::TriggerListDelegateWidget* m_calculator;
	};
}

#endif // PALAPELI_TRIGGERLISTVIEW_P_H

/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PALAPELI_TRIGGERLISTVIEW_P_H
#define PALAPELI_TRIGGERLISTVIEW_P_H

#include "triggerlistview.h"
#include "elidinglabel.h"
#include "mouseinputbutton.h"
#include <QApplication>
#include <QCollator>
#include <QHBoxLayout>
#include <QLabel>
#include <KCategorizedSortFilterProxyModel>
#include <KWidgetItemDelegate>

namespace Palapeli
{
	class TriggerListProxyModel : public KCategorizedSortFilterProxyModel
	{
		public:
			explicit TriggerListProxyModel(QObject* parent = nullptr)
				: KCategorizedSortFilterProxyModel(parent)
			{
				setCategorizedModel(true);
				m_collator.setCaseSensitivity(Qt::CaseSensitive);
			}
		protected:
			int compareCategories(const QModelIndex& left, const QModelIndex& right) const override
			{
				const int categoryLeft = left.data(KCategorizedSortFilterProxyModel::CategorySortRole).value<int>();
				const int categoryRight = right.data(KCategorizedSortFilterProxyModel::CategorySortRole).value<int>();
				return categoryRight - categoryLeft;
			}
			bool subSortLessThan(const QModelIndex& left, const QModelIndex& right) const override
			{
				const QString textLeft = left.data(Qt::DisplayRole).toString();
				const QString textRight = right.data(Qt::DisplayRole).toString();
				return m_collator.compare(textLeft, textRight) < 0;
			}
		private:
			QCollator m_collator;
	};

	class TriggerListDelegateWidget : public QWidget
	{
		Q_OBJECT
		public:
			explicit TriggerListDelegateWidget(QWidget* parent = nullptr) : QWidget(parent)
			{
				m_iconLabel = new QLabel(this);
				m_nameLabel = new Palapeli::ElidingLabel(this);
				m_inputButton = new Palapeli::MouseInputButton(this);
				connect(m_inputButton, &MouseInputButton::triggerChanged, this, &TriggerListDelegateWidget::triggerChanged);
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
			explicit TriggerListDelegate(QAbstractItemView* view, QObject* parent = nullptr) : KWidgetItemDelegate(view, parent)
			{
				m_calculator = new Palapeli::TriggerListDelegateWidget(view);
				m_calculator->setVisible(false);
			}
			void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
			{
				Q_UNUSED(index)
				QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, nullptr);
			}
			QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override
			{
				updateItemWidgets(QList<QWidget*>() << m_calculator, option, index);
				return m_calculator->minimumSizeHint();
			}
		protected:
			QList<QWidget*> createItemWidgets(const QModelIndex &index) const override
			{
				Q_UNUSED(index);
				return QList<QWidget*>() << new Palapeli::TriggerListDelegateWidget(itemView());
			}
			void updateItemWidgets(QList<QWidget*> widgets, const QStyleOptionViewItem& option, const QPersistentModelIndex& index) const override
			{
				Palapeli::TriggerListDelegateWidget* widget = qobject_cast<Palapeli::TriggerListDelegateWidget*>(widgets[0]);
				//adjust widget contents
				widget->setIcon(index.data(Qt::DecorationRole).value<QIcon>());
				widget->setText(index.data(Qt::DisplayRole).value<QString>());
				disconnect(widget, nullptr, this, nullptr);
				widget->setTrigger(index.data(TriggerRole).value<Palapeli::Trigger>());
				connect(widget, &TriggerListDelegateWidget::triggerChanged, this, &TriggerListDelegate::slotTriggerChanged);
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
				model->setData(index, QVariant::fromValue(newTrigger), Palapeli::TriggerRole);
				Q_EMIT triggerChanged();
			}
		private:
			Palapeli::TriggerListDelegateWidget* m_calculator;
	};
}

#endif // PALAPELI_TRIGGERLISTVIEW_P_H

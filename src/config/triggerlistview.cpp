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
#include "elidinglabel.h"
#include "mouseinputbutton.h"
#include "../engine/interactor.h"

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
				layout->addWidget(m_nameLabel);
				layout->addWidget(m_inputButton);
				m_inputButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
			}
			void setIcon(const QIcon& icon)
			{
				//TODO: respect global icon size configuration
				m_iconLabel->setPixmap(icon.pixmap(22));
			}
			void setText(const QString& text)
			{
				m_nameLabel->setFullText(text);
			}
			void setTrigger(const Palapeli::Trigger& trigger)
			{
				m_inputButton->setTrigger(trigger);
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
			TriggerListDelegate(QAbstractItemView* view, QObject* parent = 0) : KWidgetItemDelegate(view, parent)
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
				connect(widget, SIGNAL(triggerChanged(const Palapeli::Trigger&)), SLOT(triggerChanged(const Palapeli::Trigger&)));
				//adjust widget geometry
				QRect rect = option.rect;
				rect.moveTop(0);
				widget->setGeometry(rect);
			}
		private Q_SLOTS:
			void triggerChanged(const Palapeli::Trigger& newTrigger)
			{
				const QModelIndex index = focusedIndex();
				QAbstractItemModel* model = const_cast<QAbstractItemModel*>(index.model());
				model->setData(index, qVariantFromValue(newTrigger), Palapeli::TriggerRole);
			}
		private:
			Palapeli::TriggerListDelegateWidget* m_calculator;
	};
}

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

Palapeli::TriggerListView::TriggerListView(const QMap<QByteArray, Palapeli::Interactor*>& interactors, const QMap<QByteArray, Palapeli::Trigger>& associations, QWidget* parent)
	: KCategorizedView(parent)
	, m_categoryDrawer(new KCategoryDrawer)
	, m_baseModel(new QStandardItemModel(this))
	, m_proxyModel(new Palapeli::TriggerListProxyModel(this))
	, m_delegate(new Palapeli::TriggerListDelegate(this))
{
	//fill base model with interactors
	QMap<QByteArray, Palapeli::Interactor*>::const_iterator it1 = interactors.begin(), it2 = interactors.end();
	for (; it1 != it2; ++it1)
	{
		Palapeli::Interactor* interactor = it1.value();
		QStandardItem* item = new QStandardItem;
		item->setData(interactor->description(), Qt::DisplayRole);
		item->setData(interactor->icon(), Qt::DecorationRole);
		item->setData(it1.key(), Palapeli::InteractorRole);
		item->setData(qVariantFromValue(associations.value(it1.key())), Palapeli::TriggerRole);
		item->setData(categoryToString(interactor->category()), KCategorizedSortFilterProxyModel::CategoryDisplayRole);
		item->setData(interactor->category(), KCategorizedSortFilterProxyModel::CategorySortRole);
		m_baseModel->appendRow(item);
	}
	//setup model/view
	m_proxyModel->setSourceModel(m_baseModel);
	setModel(m_proxyModel);
	setItemDelegate(m_delegate);
// 	setCategoryDrawer(m_categoryDrawer); //FIXME: Why do I crash?
}

Palapeli::TriggerListView::~TriggerListView()
{
	delete m_categoryDrawer;
}

QMap<QByteArray, Palapeli::Trigger> Palapeli::TriggerListView::triggers() const
{
	//read triggers from base model
	QMap<QByteArray, Palapeli::Trigger> result;
	for (int i = 0; i < m_baseModel->rowCount(); ++i)
	{
		QStandardItem* item = m_baseModel->item(i);
		const QByteArray interactor = item->data(Palapeli::InteractorRole).value<QByteArray>();
		const Palapeli::Trigger trigger = item->data(Palapeli::TriggerRole).value<Palapeli::Trigger>();
		result.insert(interactor, trigger);
	}
	return result;
}

#include "triggerlistview.moc"

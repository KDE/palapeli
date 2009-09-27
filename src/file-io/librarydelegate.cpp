/***************************************************************************
 *   Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
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

#include "librarydelegate.h"
#include "librarymodel.h"
#include "puzzle.h"

#include <QAbstractItemView>
#include <QGridLayout>
#include <QLabel>
#include <QStyle>
#include <KLocalizedString>

Palapeli::LibraryDelegate::LibraryDelegate(QAbstractItemView* view)
	: KWidgetItemDelegate(view, view)
{
	view->setItemDelegate(this);
}

QList<QWidget*> Palapeli::LibraryDelegate::createItemWidgets() const
{
	//container widget
	QWidget *container = new QWidget;
	QGridLayout *layout = new QGridLayout;
	container->setLayout(layout);
	//contained widgets
	static const QSize bigIconSize(48, 48);
	static const QSize smallIconSize(32, 32);
	QLabel* thumbnailWidget = new QLabel;
	thumbnailWidget->setAlignment(Qt::AlignCenter);
	thumbnailWidget->setMaximumSize(Palapeli::Puzzle::ThumbnailBaseSize);
	thumbnailWidget->setMinimumSize(Palapeli::Puzzle::ThumbnailBaseSize);
	QLabel* headlineWidget = new QLabel;
	QLabel* sublineWidget1 = new QLabel;
	QLabel* sublineWidget2 = new QLabel;
	QFont headlineFont = headlineWidget->font();
	headlineFont.setBold(true);
	headlineWidget->setFont(headlineFont);
	//build layout
	layout->addWidget(thumbnailWidget, 0, 0, 3, 1);
	layout->addWidget(headlineWidget, 0, 1, Qt::AlignLeft | Qt::AlignBottom);
	layout->addWidget(sublineWidget1, 1, 1, Qt::AlignLeft | Qt::AlignTop);
	layout->addWidget(sublineWidget2, 2, 1, Qt::AlignLeft | Qt::AlignTop);
	return QList<QWidget *>() << container;
}

void Palapeli::LibraryDelegate::updateItemWidgets(const QList<QWidget*> widgets, const QStyleOptionViewItem& option, const QPersistentModelIndex& index) const
{
	//retrieve widgets
	QWidget* container = widgets[0];
	QGridLayout* layout = qobject_cast<QGridLayout*>(container->layout());
	QLabel* thumbnailWidget = qobject_cast<QLabel*>(layout->itemAtPosition(0, 0)->widget());
	QLabel* headlineWidget = qobject_cast<QLabel*>(layout->itemAtPosition(0, 1)->widget());
	QLabel* sublineWidget1 = qobject_cast<QLabel*>(layout->itemAtPosition(1, 1)->widget());
	QLabel* sublineWidget2 = qobject_cast<QLabel*>(layout->itemAtPosition(2, 1)->widget());
	//retrieve data
	const QString name = index.data(Palapeli::LibraryModel::NameRole).toString();
	const QString comment = index.data(Palapeli::LibraryModel::CommentRole).toString();
	const QString author = index.data(Palapeli::LibraryModel::AuthorRole).toString();
	const QPixmap thumbnail = index.data(Palapeli::LibraryModel::ThumbnailRole).value<QPixmap>();
	//update widgets
	const QString authorString = author.isEmpty() ? QString() : i18nc("Author attribution, e.g. \"By Jack\"", "By %1").arg(author);
	headlineWidget->setText(name.isEmpty() ? i18n("[No name]") : name);
	sublineWidget1->setText(comment.isEmpty() ? authorString : comment);
	sublineWidget2->setText(comment.isEmpty() ? QString() : authorString);
	thumbnailWidget->setPixmap(thumbnail);
	//update size of layout and text color
	container->setGeometry(QRect(QPoint(0, 0), option.rect.size()));
	//update font of author line
	QWidget* italicWidget = comment.isEmpty() ? sublineWidget1 : sublineWidget2;
	QWidget* nonItalicWidget = comment.isEmpty() ? sublineWidget2 : sublineWidget1;
	QFont baseFont = sublineWidget1->font();
	baseFont.setItalic(false);
	nonItalicWidget->setFont(baseFont);
	baseFont.setItalic(true);
	italicWidget->setFont(baseFont);
	//update text color in palette
	QPalette basePalette = thumbnailWidget->palette();
	if (option.state & QStyle::State_Selected)
		basePalette.setBrush(QPalette::Text, option.palette.highlightedText());
	headlineWidget->setPalette(basePalette);
	sublineWidget1->setPalette(basePalette);
	sublineWidget2->setPalette(basePalette);
}

void Palapeli::LibraryDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	//paint background for selected or hovered item
	QStyleOptionViewItemV4 opt = option;
	itemView()->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, 0);
}

QSize Palapeli::LibraryDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    //only the height of the size hint is interesting
    static const int topMargin = itemView()->style()->pixelMetric(QStyle::PM_LayoutTopMargin);
    static const int bottomMargin = itemView()->style()->pixelMetric(QStyle::PM_LayoutBottomMargin);
    static const int verticalSizeHint = Palapeli::Puzzle::ThumbnailBaseSize.height() + topMargin + bottomMargin;
    return QSize(itemView()->viewport()->width(), verticalSizeHint);
}

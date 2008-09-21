/***************************************************************************
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

#include "librarydelegate.h"
#include "library.h"

#include <QApplication>
#include <QPainter>
#include <KIcon>
#include <KLocalizedString>

//TODO: Display a "Loading..." caption or similar when the DisplayRole is not available.

Palapeli::LibraryDelegate::LibraryDelegate(QObject* parent)
	: QAbstractItemDelegate(parent)
{
}

void Palapeli::LibraryDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	//highlight selected item
	QStyleOptionViewItemV4 opt = option;
	QStyle* style = opt.widget ? opt.widget->style() : QApplication::style();
	style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);
	//block 1: draw thumbnail icon
	const QImage icon = index.data(Qt::DecorationRole).value<QImage>();
	if (!icon.isNull())
	{
		//find metrics
		const int xPadding = (Palapeli::Library::IconSize - icon.width()) / 2;
		const int yPadding = (Palapeli::Library::IconSize - icon.height()) / 2;
		int x = option.rect.left() + Margin + xPadding;
		if (option.direction == Qt::RightToLeft)
			x = option.rect.right() + xPadding - (Margin + Palapeli::Library::IconSize);
		//draw icon
		painter->save();
		painter->drawImage(QPointF(x, option.rect.y() + Margin + yPadding), icon);
		painter->restore();
	}
	//block 2: draw piece count
	static const QPixmap pieceCountIcon = KIcon("preferences-plugin").pixmap(Palapeli::Library::IconSize, QIcon::Disabled);
	const int pieceCount = index.data(Palapeli::Library::PieceCountRole).toInt();
	const QString pieceCountText = QString::number(pieceCount);
	if (!pieceCountIcon.isNull() && pieceCount != 0)
	{
		//find metrics
		int x = option.rect.right() - (Margin + Palapeli::Library::IconSize);
		if (option.direction == Qt::RightToLeft)
			x = option.rect.left() + Margin;
		//draw icon
		painter->save();
		painter->setOpacity(0.5);
		painter->drawPixmap(QPointF(x, option.rect.y() + Margin), pieceCountIcon);
		painter->restore();
		//adjust size of text to make text fit (this code assumes that the text's width is always >= its height, and that the relation between height and width is linear for different font sizes)
		painter->save();
		QFont font = option.font;
		font.setWeight(QFont::Normal);
		painter->setFont(font);
		const QFontMetrics fm(font);
		const QRect textRect = fm.boundingRect(pieceCountText);
		static const int desiredTextWidth = Palapeli::Library::IconSize / 2;
		font.setPointSizeF(font.pointSizeF() * desiredTextWidth / textRect.width());
		painter->setFont(font);
		//draw text
		painter->drawText(QRect(x, option.rect.y() + Margin, Palapeli::Library::IconSize, Palapeli::Library::IconSize), Qt::AlignCenter, pieceCountText);
		painter->restore();
	}
	//block 3: draw text between the icons
	painter->save();
	static const int iconAreaWidth = Palapeli::Library::IconSize + 2 * Margin;
	QRect boundingRect(option.rect.x() + iconAreaWidth, option.rect.y(), option.rect.width() - 2 * iconAreaWidth, option.rect.height());
	//font for first line
	QFont font = option.font;
	font.setWeight(QFont::Bold);
	painter->setFont(font);
	//first line
	const QString name = index.data(Qt::DisplayRole).toString();
	paintLine(name, boundingRect, painter, option);
	//font for second line
	font.setWeight(QFont::Normal);
	painter->setFont(font);
	//second line
	const QString comment = index.data(Palapeli::Library::CommentRole).toString();
	if (!comment.isEmpty())
		paintLine(comment, boundingRect, painter, option);
	//font for third line
	font.setStyle(QFont::StyleItalic);
	painter->setFont(font);
	//third line
	const QString author = index.model()->data(index, Palapeli::Library::AuthorRole).toString();
	if (!author.isEmpty())
		paintLine(i18nc("@item:intext", "by %1", author), boundingRect, painter, option);
	painter->restore();
}

void Palapeli::LibraryDelegate::paintLine(const QString& text, QRect& boundingRect, QPainter* painter, const QStyleOptionViewItem& option) const
{
	//set font
	const QFontMetrics fm(painter->font());
	//adjust painting position
	const QRect textRect = fm.boundingRect(text);
	int left = boundingRect.left() + Margin;
	if (option.direction == Qt::RightToLeft)
	{
		if (textRect.width() < boundingRect.width())
			left = boundingRect.right() - textRect.width() - Margin;
		else
			left = boundingRect.left() + Margin;
	}
	boundingRect.setTop(boundingRect.top() + textRect.height());
	painter->drawText(left, boundingRect.y(), fm.elidedText(text, option.textElideMode, boundingRect.width() - Margin));
}

QSize Palapeli::LibraryDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	Q_UNUSED(option)
	Q_UNUSED(index)
	static const int iconAreaSize = 2 * Margin + Palapeli::Library::IconSize;
	return QSize(100, iconAreaSize); //height is the important point, width is determined by widget bounds
}

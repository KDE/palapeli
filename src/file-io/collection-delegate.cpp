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

#include "collection-delegate.h"
#include "collection.h"
#include "puzzle-old.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <KLocalizedString>

namespace Metrics
{
	const int Padding = 6;
}

Palapeli::CollectionDelegate::CollectionDelegate(QObject* parent)
	: QStyledItemDelegate(parent)
{
	QAbstractItemView* view = qobject_cast<QAbstractItemView*>(parent);
	if (view)
		view->setItemDelegate(this);
}

QRect Palapeli::CollectionDelegate::thumbnailRect(const QRect& baseRect) const
{
	QRect thumbnailBaseRect(QPoint(Metrics::Padding + baseRect.left(), 0), Palapeli::PuzzleMetadata::ThumbnailBaseSize);
	thumbnailBaseRect.moveCenter(QPoint(thumbnailBaseRect.center().x(), baseRect.center().y()));
	if (QApplication::isRightToLeft())
		thumbnailBaseRect.moveRight(baseRect.right() - Metrics::Padding);
	return thumbnailBaseRect;
}

void Palapeli::CollectionDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	paintItem(painter, option, index);
}

void Palapeli::CollectionDelegate::paintItem(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	const bool rtl = option.direction == Qt::RightToLeft;
	QRect baseRect = option.rect;
	//draw background
	QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, 0);
	//draw thumbnail
	QRect thumbnailBaseRect = this->thumbnailRect(baseRect);
	const QPixmap thumbnail = index.data(Palapeli::Collection::ThumbnailRole).value<QPixmap>();
	QRect thumbnailRect(thumbnailBaseRect.topLeft(), thumbnail.size());
	thumbnailRect.translate( //center inside thumbnailBaseRect
		(thumbnailBaseRect.width() - thumbnailRect.width()) / 2,
		(thumbnailBaseRect.height() - thumbnailRect.height()) / 2
	);
	painter->drawPixmap(thumbnailRect.topLeft(), thumbnail);
	//find metrics: text
	QStringList texts; QList<QFont> fonts;
	{
		QString name = index.data(Palapeli::Collection::NameRole).toString();
		const int pieceCount = index.data(Palapeli::Collection::PieceCountRole).toInt();
		if (name.isEmpty())
			name = i18n("[No name]");
		name = ki18ncp("Puzzle description, %2 = name string, %1 = piece count", "%2 (%1 piece)", "%2 (%1 pieces)")
		       .subs(pieceCount).subs(name).toString();
		texts << name;
		QFont theFont(painter->font()); theFont.setBold(true); fonts << theFont;
	}{
		QString comment = index.data(Palapeli::Collection::CommentRole).toString();
		if (!comment.isEmpty())
		{
			texts << comment;
			fonts << painter->font();
		}
	}{
		QString author = index.data(Palapeli::Collection::AuthorRole).toString();
		if (!author.isEmpty())
		{
			const QString authorString = ki18nc("Author attribution, e.g. \"by Jack\"", "by %1").subs(author).toString();
			texts << authorString;
			QFont theFont(painter->font()); theFont.setItalic(true); fonts << theFont;
		}
	}
	QList<QRect> textRects; int totalTextHeight = 0;
	for (int i = 0; i < texts.count(); ++i)
	{
		QFontMetrics fm(fonts[i]);
		textRects << fm.boundingRect(texts[i]);
		textRects[i].setHeight(qMax(textRects[i].height(), fm.lineSpacing()));
		totalTextHeight += textRects[i].height();
	}
	QRect textBaseRect(baseRect);
	if (rtl)
	{
		textBaseRect.moveRight(thumbnailBaseRect.left() - Metrics::Padding);
		textBaseRect.adjust(Metrics::Padding, Metrics::Padding, 0, -Metrics::Padding);
	}
	else
	{
		textBaseRect.moveLeft(thumbnailBaseRect.right() + Metrics::Padding);
		textBaseRect.adjust(0, Metrics::Padding, -Metrics::Padding, -Metrics::Padding);
	}
	textBaseRect.setHeight(totalTextHeight);
	textBaseRect.moveTop(baseRect.top() + (baseRect.height() - textBaseRect.height()) / 2);
	//draw texts
	QRect currentTextRect(textBaseRect);
	painter->save();
	for (int i = 0; i < texts.count(); ++i)
	{
		painter->setFont(fonts[i]);
		const QRect& textRect = textRects[i];
		currentTextRect.setHeight(textRect.height());
		painter->drawText(currentTextRect, Qt::AlignLeft | Qt::AlignVCenter, texts[i]);
		currentTextRect.moveTop(currentTextRect.bottom());
	}
	painter->restore();
}

void Palapeli::CollectionDelegate::paintHeader(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QStyledItemDelegate::paint(painter, option, index);
}

QSize Palapeli::CollectionDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	Q_UNUSED(option) Q_UNUSED(index)
	//TODO: take text size into account
	return QSize(400, Palapeli::PuzzleMetadata::ThumbnailBaseSize.height() + 2 * Metrics::Padding);
}

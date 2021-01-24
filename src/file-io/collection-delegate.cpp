/*
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "collection-delegate.h"
#include "collection.h"
#include "puzzlestructs.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <KLocalizedString>

const QSize Palapeli::PuzzleMetadata::ThumbnailBaseSize(64, 64);

namespace Metrics
{
    const int Padding = 6;
}

Palapeli::CollectionDelegate::CollectionDelegate (QObject* parent)
        : QStyledItemDelegate(parent)
{
    QAbstractItemView* view = qobject_cast<QAbstractItemView*>(parent);
    if (view) {
        view->setItemDelegate(this);
        m_viewport = view->viewport();
    }
}

QRect Palapeli::CollectionDelegate::thumbnailRect (const QRect& baseRect) const
{
    QRect thumbnailBaseRect (QPoint(Metrics::Padding + baseRect.left(), 0),
                             Palapeli::PuzzleMetadata::ThumbnailBaseSize);
    thumbnailBaseRect.moveCenter (QPoint(thumbnailBaseRect.center().x(),
                                  baseRect.center().y()));
    if (QApplication::isRightToLeft()) {
        thumbnailBaseRect.moveRight (baseRect.right() - Metrics::Padding);
    }
    return thumbnailBaseRect;
}

void Palapeli::CollectionDelegate::paint (
                                        QPainter* painter,
                                        const QStyleOptionViewItem& option,
                                        const QModelIndex& index) const
{
    QRect baseRect = option.rect;	// Total space available for item.

    // Calculate item's column number in list-view. Add 1 in odd-numbered rows.
    int nItemsPerRow = qMax (m_viewport->width() / baseRect.width(), 1);
    int oddColumn = index.row() % nItemsPerRow;
    oddColumn = oddColumn + ((index.row() / nItemsPerRow) % 2);

    // Draw background of item.
    QColor bkgColor;
    if (option.state & QStyle::State_Selected) {
        bkgColor = option.palette.color (QPalette::Highlight);
    } else if (oddColumn % 2) {
        // The shading alternates along each row in the list view and
        // each odd-numbered row starts with a shaded item, so we have
        // a checkerboard pattern, regardless of whether nItemsPerRow
        // is odd or even (see the above calculation).
        bkgColor = option.palette.color (QPalette::AlternateBase);
    } else {
        bkgColor = option.palette.color (QPalette::Base);
    }
    painter->fillRect (option.rect, bkgColor);

    // Draw thumbnail.
    QRect thumbnailBaseRect = this->thumbnailRect (baseRect);
    const QPixmap thumbnail = index.data (Palapeli::Collection::ThumbnailRole)
                                          .value<QPixmap>();
    QRect thumbnailRect (thumbnailBaseRect.topLeft(), thumbnail.size());
    thumbnailRect.translate (		// Center inside thumbnailBaseRect.
                (thumbnailBaseRect.width()  - thumbnailRect.width()) / 2,
                (thumbnailBaseRect.height() - thumbnailRect.height()) / 2);
    painter->drawPixmap (thumbnailRect.topLeft(), thumbnail);

    // Calculate the maximum space available for text lines.
    QRect textBaseRect (baseRect);
    textBaseRect.setWidth (baseRect.width() - thumbnailBaseRect.width()
                                            - 2*Metrics::Padding);
    if (option.direction == Qt::RightToLeft) {
        textBaseRect.moveRight (thumbnailBaseRect.left() - Metrics::Padding);
        textBaseRect.adjust (Metrics::Padding, Metrics::Padding,
                             0, -Metrics::Padding);
    }
    else {
        textBaseRect.moveLeft (thumbnailBaseRect.right() + Metrics::Padding);
        textBaseRect.adjust (0, Metrics::Padding,
                             -Metrics::Padding, -Metrics::Padding);
    }

    // Find the contents and sizes for the text lines.
    QStringList texts; QList<QFont> fonts;
    {
        QString name = index.data(Palapeli::Collection::NameRole).toString();
        const int pieceCount = index.data (Palapeli::Collection::PieceCountRole)
                                           .toInt();
        if (name.isEmpty()) {
            name = i18n("[No name]");
        }
        if (pieceCount > 0) {
            name = ki18ncp (
                    "Puzzle description, %2 = name string, %1 = piece count",
                    "%2 (%1 piece)",
                    "%2 (%1 pieces)")
                    .subs(pieceCount).subs(name).toString();
        }
        texts << name;
        QFont theFont (painter->font());
        theFont.setBold(true);
        fonts << theFont;
    }{
        QString comment = index.data (Palapeli::Collection::CommentRole)
                                      .toString();
        if (!comment.isEmpty()) {
            texts << comment;
            fonts << painter->font();
        }
    }{
        QString author = index.data (Palapeli::Collection::AuthorRole)
                                     .toString();
        if (!author.isEmpty()) {
            const QString authorString = ki18nc (
                        "Author attribution, e.g. \"by Jack\"",
                        "by %1")
                        .subs(author).toString();
            texts << authorString;
            QFont theFont (painter->font());
            theFont.setItalic(true);
            fonts << theFont;
        }
    }
    QList<QRect> textRects;
    int totalTextHeight = 0;
    QRect maxRect (QPoint(0, 0), textBaseRect.size());
    for (int i = 0; i < texts.count(); ++i) {
        QFontMetrics fm(fonts[i]);
        textRects << fm.boundingRect (maxRect, Qt::AlignLeft | Qt::AlignTop |
                                      Qt::TextWordWrap, texts[i]);
        totalTextHeight += textRects[i].height();
    }

    // Vertically center however many text lines there are.
    textBaseRect.setHeight (totalTextHeight);
    textBaseRect.moveTop (baseRect.top() +
                          (baseRect.height() - textBaseRect.height()) / 2);

    // Draw the text lines.
    QRect currentTextRect (textBaseRect);
    painter->save();
    for (int i = 0; i < texts.count(); ++i) {
        painter->setFont(fonts[i]);
        const QRect& textRect = textRects[i];
        currentTextRect.setHeight(textRect.height());
        painter->drawText (currentTextRect,
                           Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                           texts[i]);
        currentTextRect.moveTop (currentTextRect.bottom());
    }
    painter->restore();
}

QSize Palapeli::CollectionDelegate::sizeHint (
                                        const QStyleOptionViewItem& option,
                                        const QModelIndex& index) const
{
    Q_UNUSED(index)

    // Fit a varying number of columns into the list-view.
    int minWidth = 4 * (Palapeli::PuzzleMetadata::ThumbnailBaseSize.width() +
                        Metrics::Padding);
    int viewportWidth = m_viewport->width();
    int hintWidth     = minWidth;
    int nItemsPerRow  = viewportWidth / minWidth;

    // Expand the hinted width, so that the columns will fill the viewport.
    if (nItemsPerRow > 0) {
        // The 0:1 adjustment works around a graphics glitch, when nItemsPerRow
        // exactly divides viewportWidth and instead of nItemsPerRow columns
        // we suddenly get one column less, plus a large empty space, even
        // though QListView::spacing() is zero.
        viewportWidth = viewportWidth - ((viewportWidth % nItemsPerRow) ? 0:1);
        hintWidth     = viewportWidth / nItemsPerRow;
    }

    // Set the height to contain the thumbnail or 4 lines of text.
    int hintHeight    = Palapeli::PuzzleMetadata::ThumbnailBaseSize.height();
    int fontHeight = option.fontMetrics.height();
    if (hintHeight < fontHeight*4) {
        hintHeight = fontHeight*4;
    }

    return QSize(hintWidth, hintHeight + 2 * Metrics::Padding);
}

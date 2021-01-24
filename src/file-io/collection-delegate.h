/*
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELI_COLLECTIONDELEGATE_H
#define PALAPELI_COLLECTIONDELEGATE_H

#include <QStyledItemDelegate>

namespace Palapeli
{
    class CollectionDelegate : public QStyledItemDelegate
    {
    public:
        explicit CollectionDelegate     (QObject* parent = nullptr);

        void paint     (QPainter* painter,
                                const QStyleOptionViewItem& option,
                                const QModelIndex& index) const override;
        QSize sizeHint (const QStyleOptionViewItem& option,
                                const QModelIndex& index) const override;

    private:
        QRect thumbnailRect    (const QRect& baseRect) const;
	QWidget * m_viewport;
    };
}

#endif // PALAPELI_COLLECTIONDELEGATE_H

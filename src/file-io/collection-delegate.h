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

#ifndef PALAPELI_COLLECTIONDELEGATE_H
#define PALAPELI_COLLECTIONDELEGATE_H

#include <QStyledItemDelegate>

namespace Palapeli
{
    class CollectionDelegate : public QStyledItemDelegate
    {
    public:
        CollectionDelegate     (QObject* parent = 0);

        void paint     (QPainter* painter,
                                const QStyleOptionViewItem& option,
                                const QModelIndex& index) const Q_DECL_OVERRIDE;
        QSize sizeHint (const QStyleOptionViewItem& option,
                                const QModelIndex& index) const Q_DECL_OVERRIDE;

    private:
        QRect thumbnailRect    (const QRect& baseRect) const;
	QWidget * m_viewport;
    };
}

#endif // PALAPELI_COLLECTIONDELEGATE_H

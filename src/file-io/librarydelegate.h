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

#ifndef PALAPELI_LIBRARYDELEGATE_H
#define PALAPELI_LIBRARYDELEGATE_H

#include <KWidgetItemDelegate>

namespace Palapeli
{
	class LibraryDelegate : public KWidgetItemDelegate
	{
		public:
			explicit LibraryDelegate(QAbstractItemView* view);

			virtual QList<QWidget*> createItemWidgets() const;
			virtual void updateItemWidgets(const QList<QWidget*> widgets, const QStyleOptionViewItem& option, const QPersistentModelIndex& index) const;
			virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
			virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
	};
}

#endif // PALAPELI_LIBRARYDELEGATE_H

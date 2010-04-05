/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef PALAPELI_PIECE_P_H
#define PALAPELI_PIECE_P_H

#include <QGraphicsItem>
#include <QObject>
#include <QStyleOptionGraphicsItem>

namespace Palapeli
{
	class SelectionAwarePixmapItem : public QObject, public QGraphicsPixmapItem
	{
		Q_OBJECT
		public:
			explicit SelectionAwarePixmapItem(const QPixmap& pixmap, QGraphicsItem* parent = 0)
				: QGraphicsPixmapItem(pixmap, parent)
			{
			}

			virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
			{
				//suppress the ugly selection outline (we have much nicer selection indications in Palapeli::Piece)
				QStyleOptionGraphicsItem opt(*option);
				opt.state = opt.state & ~QStyle::State_Selected;
				QGraphicsPixmapItem::paint(painter, &opt, widget);
			}
		Q_SIGNALS:
			void selectedChanged(bool selected);
		protected:
			virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value)
			{
				if (change == ItemSelectedChange)
					emit selectedChanged(value.toBool());
				return QGraphicsPixmapItem::itemChange(change, value);
			}
	};
}

#endif // PALAPELI_PIECE_P_H

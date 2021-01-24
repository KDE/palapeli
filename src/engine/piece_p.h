/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
			explicit SelectionAwarePixmapItem(const QPixmap& pixmap, QGraphicsItem* parent = nullptr)
				: QGraphicsPixmapItem(pixmap, parent)
			{
			}

			void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override
			{
				//suppress the ugly selection outline (we have much nicer selection indications in Palapeli::Piece)
				QStyleOptionGraphicsItem opt(*option);
				opt.state = opt.state & ~QStyle::State_Selected;
				QGraphicsPixmapItem::paint(painter, &opt, widget);
			}
		Q_SIGNALS:
			void selectedChanged(bool selected);
		protected:
			QVariant itemChange(GraphicsItemChange change, const QVariant& value) override
			{
				if (change == ItemSelectedChange)
					Q_EMIT selectedChanged(value.toBool());
				return QGraphicsPixmapItem::itemChange(change, value);
			}
	};
}

#endif // PALAPELI_PIECE_P_H

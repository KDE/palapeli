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

#ifndef PALAPELI_AUTOSCALINGITEM_H
#define PALAPELI_AUTOSCALINGITEM_H

#include <QGraphicsItem>
class QGraphicsView;
#include <QObject>
#include <QPointF>

namespace Palapeli
{

	//This class requires that the viewport is not rotated or sheared, only translated or scaled.
	class AutoscalingItem : public QObject, public QGraphicsItem
	{
		Q_OBJECT
		public:
			AutoscalingItem(QGraphicsView* view);

			virtual QRectF boundingRect() const { return QRectF(); }
			virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* = 0) {}
		public Q_SLOTS:
			void updateScalingFactor();
		private:
			QPointF m_currentScalingFactor;
			QGraphicsView* m_view;
	};

}

#endif // PALAPELI_AUTOSCALINGITEM_H
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

#ifndef PALAPELI_CONSTRAINTVISUALIZER_P_H
#define PALAPELI_CONSTRAINTVISUALIZER_P_H

#include <QCursor>
#include <QGraphicsItem>
#include <QPropertyAnimation>

namespace Palapeli
{
	class CvHandleItem : public QObject, public QGraphicsPathItem
	{
		Q_OBJECT
		Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
		public:
			CvHandleItem(const QCursor& cursor, const QColor& baseColor, QGraphicsItem* parent = 0);

			qreal opacity() const;
			void setOpacity(qreal opacity);
			void setOpacityAnimated(qreal opacity);
		protected:
			virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
			virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
			virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
			virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);
			virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value);
		private:
			QColor m_baseColor;
			qreal m_opacity;
			QPropertyAnimation* m_animator;
	};
}

#endif // PALAPELI_CONSTRAINTVISUALIZER_P_H

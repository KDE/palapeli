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

#ifndef PALAPELI_INACCESSIBLEAREASHELPER_H
#define PALAPELI_INACCESSIBLEAREASHELPER_H

class QGraphicsRectItem;
class QGraphicsView;
#include <QObject>
class QPropertyAnimation;
#include <QRectF>
#include <QVector>

namespace Palapeli
{
	class InaccessibleAreasHelper : public QObject
	{
		Q_OBJECT
		Q_PROPERTY(qreal Opacity READ opacity WRITE setOpacity)
		public:
			InaccessibleAreasHelper(QGraphicsView* view);

			bool isActive() const;
			qreal opacity() const;
		public Q_SLOTS:
			void setActive(bool active);
			void setOpacity(qreal opacity);
			void update();
		protected:
			virtual bool eventFilter(QObject* sender, QEvent* event);
		private:
			enum Position { LeftPos = 0, RightPos, TopPos, BottomPos, PositionCount };

			QGraphicsView* m_view;
			bool m_active; qreal m_opacity;

			QVector<QGraphicsRectItem*> m_items;
			QRectF m_viewportRect;
			QPropertyAnimation* m_animator;
	};
}

#endif // PALAPELI_INACCESSIBLEAREASHELPER_H

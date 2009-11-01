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

#ifndef PALAPELI_CONSTRAINTVISUALIZER_H
#define PALAPELI_CONSTRAINTVISUALIZER_H

#include "basics.h"

class QGraphicsView;
class QPropertyAnimation;
#include <QVector>

namespace Palapeli
{
	class ConstraintVisualizer : public Palapeli::GraphicsObject<Palapeli::ConstraintVisualizerUserType>
	{
		Q_OBJECT
		public:
			ConstraintVisualizer(QGraphicsView* view);

			bool isActive() const;
		public Q_SLOTS:
			void setActive(bool active);
			void update();
		protected:
			virtual bool eventFilter(QObject* sender, QEvent* event);
		private:
			enum Position { LeftPos = 0, RightPos, TopPos, BottomPos, PositionCount };

			QGraphicsView* m_view;
			bool m_active;

			QVector<QGraphicsRectItem*> m_items;
			QRectF m_viewportRect;
			QPropertyAnimation* m_animator;
	};
}

#endif // PALAPELI_CONSTRAINTVISUALIZER_H

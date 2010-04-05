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

#ifndef PALAPELI_CONSTRAINTINTERACTOR_H
#define PALAPELI_CONSTRAINTINTERACTOR_H

#include "interactor.h"

namespace Palapeli
{
	class ConstraintInteractor : public Palapeli::Interactor
	{
		public:
			ConstraintInteractor(QGraphicsView* view);
		protected:
			virtual bool acceptMousePosition(const QPoint& pos);
			virtual void mousePressEvent(const Palapeli::MouseEvent& event);
			virtual void mouseMoveEvent(const Palapeli::MouseEvent& event);
			virtual void mouseReleaseEvent(const Palapeli::MouseEvent& event);
		private:
			enum Side { LeftSide = 0, RightSide, TopSide, BottomSide };
			QList<Side> touchingSides(const QPointF& scenePos) const;

			QList<Side> m_draggingSides;
			QPointF m_baseSceneRectOffset;
	};
}

#endif // PALAPELI_CONSTRAINTINTERACTOR_H

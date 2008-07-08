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

#ifndef PALADESIGN_POINTS_H
#define PALADESIGN_POINTS_H

class QPainter;
#include <QPair>
#include <QPointF>
#include <QRectF>
class KSvgRenderer;

namespace Paladesign
{

	class Manager;

	struct Point
	{
		QPointF position;
		int angle; //in degrees

		Point(const QPointF& pointPosition = QPointF(), int pointAngle = 0);
		void paint(QPainter* painter, KSvgRenderer* shape, const QRectF& shapeRect) const;
	};

	class Points
	{
		public:
			Points(Manager* manager);

			void paint(QPainter* painter, const QRectF& clipRect);

			static const qreal MaximumSelectionDistance = 0.1;
			static const int ItemRange = 5; //number of points displayed on each side of the points array
		protected:
//			QPair<int, int> paint(QPainter* painter, int xCoordinate, const QRectF& clipRect);
		private:
			Manager* m_manager;
	};

}

#endif // PALADESIGN_POINTS_H

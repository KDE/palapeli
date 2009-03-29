/***************************************************************************
 *   Copyright 2008 Stefan Majewsky <majewsky@gmx.net>
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

#include "mouseinteractor.h"

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

	class Points : public MouseInteractor
	{
		Q_OBJECT
		Q_PROPERTY(QString patternName READ patternName WRITE setPatternName)
		public:
			Points(Manager* manager);

			QString patternName() const;
			void setPatternName(const QString& name);

			void paint(QPainter* painter, const QRectF& clipRect);
			virtual bool hoverAreaContains(const QPointF& point);

			static const qreal MaximumSelectionDistance = 0.1;
			static const int ItemRange = 3; //number of points displayed on each side of the points array
		protected:
			virtual void mouseDown();
			virtual void mouseMove();
			virtual void mouseUp();
		private:
			Manager* m_manager;
			QString m_patternName;
	};

}

#endif // PALADESIGN_POINTS_H

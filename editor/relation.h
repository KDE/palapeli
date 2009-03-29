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

#ifndef PALADESIGN_RELATION_H
#define PALADESIGN_RELATION_H

#include "mouseinteractor.h"

#include <QLineF>
class QPainter;

namespace Paladesign
{

	class Manager;
	struct Point;

	class Relation : public MouseInteractor
	{
		public:
			Relation();

			QLineF line() const;
			virtual Point next(const Point& point, int steps = 1) const = 0;
			Point previous(const Point& point, int steps = 1) const;

			virtual bool hoverAreaContains(const QPointF& point);
			virtual void paint(QPainter* painter) const = 0;
	};

	class PhysicalRelation : public Relation
	{
		Q_OBJECT
		Q_PROPERTY(int angleStep READ angleStep WRITE setAngleStep)
		Q_PROPERTY(int angle READ angle WRITE setAngle)
		Q_PROPERTY(qreal distance READ distance WRITE setDistance)
		public:
			PhysicalRelation(qreal distance, int angle, int angleStep);

			int angle() const;      //pitch of axis
			int angleStep() const;  //difference between rotation angles of pieces
			qreal distance() const; //distance between two pieces
			QPointF positionStep() const;
			void setAngle(int angle);
			void setAngleStep(int angleStep);
			void setDistance(qreal distance);

			virtual Point next(const Point& point, int steps = 1) const;
			virtual void paint(QPainter* painter) const;
		protected:
			virtual void mouseDown();
			virtual void mouseMove();
			virtual void mouseUp();
			void precalculatePositionStep();
		private:
			//m_start... = values at the beginning of a drag operation
			int m_angleStep;
			int m_angle, m_startAngle;
			qreal m_distance, m_startDistance;
			//cached values for position step (depends on m_angle and m_distance)
			QPointF m_positionStep;
	};

	class LogicalRelation : public Relation
	{
		Q_OBJECT
		Q_PROPERTY(int relation1Step READ relation1Step WRITE setRelation1Step)
		Q_PROPERTY(int relation2Step READ relation2Step WRITE setRelation2Step)
		public:
			LogicalRelation(int relation1Step, int relation2Step, Manager* manager);

			int relation1Step() const;
			int relation2Step() const;
			void setRelation1Step(int step);
			void setRelation2Step(int step);

			virtual Point next(const Point& point, int steps = 1) const;
			virtual bool clickAreaContains(const QPointF& point);
			virtual void paint(QPainter* painter) const;
		protected:
			virtual void mouseDown();
			virtual void mouseMove();
			virtual void mouseUp();
		private:
			Manager* m_manager;
			int m_relation1Step, m_relation2Step;
	};

}

#endif // PALADESIGN_RELATION_H

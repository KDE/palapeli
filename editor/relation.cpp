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

#include "relation.h"
#include "algebra.h"
#include "manager.h"
#include "points.h"

#include <cmath>
#include <QPainter>

template<int precision> qreal pdRound(qreal value)
{
	static const qreal factor = pow(10.0, precision);
	return qreal(qRound(value * factor)) / factor;
}

////////////////////////////////////////
// Relation implementation            //
////////////////////////////////////////

Paladesign::Relation::Relation()
{
}

QLineF Paladesign::Relation::line() const
{
	static const Point center(QPointF(0.0, 0.0), 0);
	return QLineF(previous(center).position, next(center).position);
}

Paladesign::Point Paladesign::Relation::previous(const Point& point, int steps) const
{
	return next(point, -steps);
}

bool Paladesign::Relation::hoverAreaContains(const QPointF& point)
{
	return Paladesign::Algebra::distance(point, line()) <= Paladesign::Points::MaximumSelectionDistance / 2.0;
}

////////////////////////////////////////
// PhysicalRelation implementation    //
////////////////////////////////////////

Paladesign::PhysicalRelation::PhysicalRelation(qreal distance, int angle, int angleStep)
	: m_angleStep(angleStep)
	, m_angle(angle)
	, m_distance(distance)
{
	precalculatePositionStep();
}

int Paladesign::PhysicalRelation::angle() const
{
	return m_angle;
}

int Paladesign::PhysicalRelation::angleStep() const
{
	return m_angleStep;
}

qreal Paladesign::PhysicalRelation::distance() const
{
	return m_distance;
}

QPointF Paladesign::PhysicalRelation::positionStep() const
{
	return m_positionStep;
}

void Paladesign::PhysicalRelation::setAngle(int angle)
{
	m_angle = (angle + 179) % 360 - 179; //codomain of this expression: (-180, 180] degrees (i.e. 180 deg is included, but -180 deg isn't)
	precalculatePositionStep();
	announceInteractorChanges();
}

void Paladesign::PhysicalRelation::setAngleStep(int angleStep)
{
	m_angleStep = (angleStep + 179) % 360 - 179; //see Paladesign::PhysicalRelation::setAngle
	announceInteractorChanges();
}

void Paladesign::PhysicalRelation::setDistance(qreal distance)
{
	m_distance = pdRound<2>(distance);
	if (m_distance < 5 * Paladesign::Points::MaximumSelectionDistance)
		m_distance = 5 * Paladesign::Points::MaximumSelectionDistance;
	precalculatePositionStep();
	announceInteractorChanges();
}

void Paladesign::PhysicalRelation::precalculatePositionStep()
{
	static const qreal pi = acos(-1);
	const qreal radiantAngle = qreal(m_angle) / 180.0 * pi;
	m_positionStep.setX(m_distance * cos(radiantAngle));
	m_positionStep.setY(m_distance * sin(radiantAngle));
}

Paladesign::Point Paladesign::PhysicalRelation::next(const Point& point, int steps) const
{
	return Paladesign::Point(point.position + steps * m_positionStep, point.angle + steps * m_angleStep);
}

void Paladesign::PhysicalRelation::paint(QPainter* painter) const
{
	static QColor relationColor(Qt::red);
	static QColor hoveredColor(Qt::red);
	hoveredColor.setAlpha(128);
	painter->save();
	//draw implied axis (it should be so long that the ends do almost never appear on the screen)
	painter->setPen(relationColor);
	static const qreal targetCoordinate = 40; //11 = length of view edges
	const QPointF lineEnd = targetCoordinate * m_positionStep / m_distance;
	painter->drawLine(lineEnd, -lineEnd);
	//draw selection
	if (clicked())
	{
		painter->setPen(QPen(relationColor, Paladesign::Points::MaximumSelectionDistance));
		painter->drawLine(lineEnd, -lineEnd);
	}
	else if (selected() || hovered())
	{
		painter->setPen(QPen(hoveredColor, Paladesign::Points::MaximumSelectionDistance));
		painter->drawLine(lineEnd, -lineEnd);
	}
	//finished
	painter->restore();
}

void Paladesign::PhysicalRelation::mouseDown()
{
	m_startAngle = m_angle;
	m_startDistance = m_distance;
}

void Paladesign::PhysicalRelation::mouseUp()
{
}

void Paladesign::PhysicalRelation::mouseMove()
{
	const QPointF currentPoint = mousePosition();
	const qreal currentLength = Paladesign::Algebra::vectorLength(currentPoint);
	const QPointF startPoint = mouseStartPosition();
	const qreal startLength = Paladesign::Algebra::vectorLength(startPoint);
	//change length of step according to mouse movement (but with a lower threshold to ensure that the axis remains selectable)
	m_distance = pdRound<2>(currentLength / startLength * m_startDistance);
	if (m_distance < 5 * Paladesign::Points::MaximumSelectionDistance)
		m_distance = 5 * Paladesign::Points::MaximumSelectionDistance;
	//change angle
	static const qreal pi = acos(-1);
	setAngle(atan2(currentPoint.y(), currentPoint.x()) * 180.0 / pi);
}

////////////////////////////////////////
// LogicalRelation implementation     //
////////////////////////////////////////

Paladesign::LogicalRelation::LogicalRelation(int relation1Step, int relation2Step, Paladesign::Manager* manager)
	: m_manager(manager)
	, m_relation1Step(relation1Step)
	, m_relation2Step(relation2Step)
{
	//ensure that the axis does not collapse into one point (or point in the same direction as a physical relation)
	if (m_relation1Step == 0)
		m_relation1Step = 1;
	if (m_relation2Step == 0)
		m_relation2Step = 1;
}

int Paladesign::LogicalRelation::relation1Step() const
{
	return m_relation1Step;
}

int Paladesign::LogicalRelation::relation2Step() const
{
	return m_relation2Step;
}

void Paladesign::LogicalRelation::setRelation1Step(int step)
{
	m_relation1Step = step;
	if (m_relation1Step == 0)
		m_relation1Step = 1;
	announceInteractorChanges();
}

void Paladesign::LogicalRelation::setRelation2Step(int step)
{
	m_relation2Step = step;
	if (m_relation2Step == 0)
		m_relation2Step = 1;
	announceInteractorChanges();
}

Paladesign::Point Paladesign::LogicalRelation::next(const Paladesign::Point& point, int steps) const
{
	Paladesign::Relation* relation1 = m_manager->relation(0);
	Paladesign::Relation* relation2 = m_manager->relation(1);
	return relation1->next(relation2->next(point, steps * m_relation2Step), steps * m_relation1Step);
}

bool Paladesign::LogicalRelation::clickAreaContains(const QPointF& point)
{
	static const Paladesign::Point nullPoint(QPointF(0.0, 0.0), 0);
	return Paladesign::Algebra::vectorLength(next(nullPoint).position - point) < 2 * Paladesign::Points::MaximumSelectionDistance;
}

void Paladesign::LogicalRelation::paint(QPainter* painter) const
{
	static QColor relationColor(Qt::blue);
	static QColor hoveredColor(Qt::blue);
	hoveredColor.setAlpha(128);
	painter->save();
	//get metrics of a position step
	static const Paladesign::Point nullPoint(QPointF(0.0, 0.0), 0);
	const QPointF point = next(nullPoint).position;
	const qreal distance = Paladesign::Algebra::vectorLength(point);
	//draw implied axis (it should be so long that the ends do almost never appear on the screen)
	painter->setPen(relationColor);
	static const qreal targetCoordinate = 40; //11 = length of view edges
	const QPointF lineEnd = targetCoordinate * point / distance;
	painter->drawLine(lineEnd, -lineEnd);
	//draw selection
	static const qreal radius = Paladesign::Points::MaximumSelectionDistance; //shorter name
	if (clicked())
	{
		painter->setPen(relationColor);
		painter->drawEllipse(point, 2 * radius, 2 * radius);
		painter->setPen(QPen(relationColor, radius));
		painter->drawLine(lineEnd, -lineEnd);
	}
	else if (selected() || hovered())
	{
		painter->setPen(hoveredColor);
		painter->drawEllipse(point, 2 * radius, 2 * radius);
		painter->setPen(QPen(hoveredColor, radius));
		painter->drawLine(lineEnd, -lineEnd);
	}
	//finished
	painter->restore();
}

void Paladesign::LogicalRelation::mouseDown()
{
}

void Paladesign::LogicalRelation::mouseMove()
{
	const QPointF point = mousePosition();
	//find the "coordinates" of this point (in the basis formed by the physical relations)
	Paladesign::PhysicalRelation* relation1 = qobject_cast<Paladesign::PhysicalRelation*>(m_manager->relation(0));
	Paladesign::PhysicalRelation* relation2 = qobject_cast<Paladesign::PhysicalRelation*>(m_manager->relation(1));
	const QPointF direction1 = relation1->positionStep();
	const QPointF direction2 = relation2->positionStep();
	// !direction1 = the vector that is orthogonal to direction1
	const qreal coordinate1 = (point * !direction2) / (direction1 * !direction2);
	const qreal coordinate2 = (point * !direction1) / (direction2 * !direction1);
	int coordinate1Int = qRound(coordinate1);
	int coordinate2Int = qRound(coordinate2);
	//ensure that the relation is not aligned with the physical axes
	if (coordinate1Int == 0)
		coordinate1Int = (coordinate1 == 0) ? 1 : (coordinate1 / qAbs(coordinate1)); // = +/-1
	if (coordinate2Int == 0)
		coordinate2Int = (coordinate2 == 0) ? 1 : (coordinate2 / qAbs(coordinate2));
	//use greatest common divisor to find simplier representations of this axis angle
	int gcd = Paladesign::Algebra::greatestCommonDivisor(coordinate1Int, coordinate2Int);
	coordinate1Int /= gcd;
	coordinate2Int /= gcd;
	//set coordinates if something has changed
	if (coordinate1Int != m_relation1Step || coordinate2Int != m_relation2Step)
	{
		m_relation1Step = coordinate1Int;
		m_relation2Step = coordinate2Int;
		announceInteractorChanges();
	}
}

void Paladesign::LogicalRelation::mouseUp()
{
}

#include "relation.moc"

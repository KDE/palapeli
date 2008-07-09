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

#include "points.h"
#include "manager.h"
#include "relation.h"
#include "shapes.h"

#include <QPainter>
#include <KSvgRenderer>

Paladesign::Point::Point(const QPointF& pointPosition, int pointAngle)
	: position(pointPosition)
	, angle(pointAngle)
{
}

void Paladesign::Point::paint(QPainter* painter, KSvgRenderer* shape, const QRectF& shapeRect) const
{
	painter->save();
	painter->translate(position);
	painter->rotate(angle);
	//draw point
	static const qreal pointRadius = 0.05; //in logical coordinates!
	static const QPointF center(0.0, 0.0);
	static const QPointF top(0.0, -5.0 * pointRadius);
	painter->drawLine(center, top);
	painter->drawEllipse(center, pointRadius, pointRadius);
	//draw shape
	static const QString elementId = QLatin1String("paladesign-shape");
	painter->setOpacity(0.3);
	painter->setBackgroundMode(Qt::TransparentMode);
	shape->render(painter, elementId, shapeRect);
	painter->restore();
}

Paladesign::Points::Points(Paladesign::Manager* manager)
	: m_manager(manager)
{
}

void Paladesign::Points::paint(QPainter* painter, const QRectF& clipRect)
{
	Q_UNUSED(clipRect)
	static QColor pointColor(Qt::black);
	static const Paladesign::Point basePoint(QPointF(0.0, 0.0), 0);
	//draw relations as the implied axes
	for (int i = 0; i < m_manager->relationCount(); ++i)
		m_manager->relation(i)->paint(painter);
	//draw points
	painter->save();
	for (int x = -3; x <= 3; ++x)
	{
		Paladesign::Point basePoint2 = m_manager->relation(0)->next(basePoint, x);
		for (int y = -3; y <= 3; ++y)
		{
			Paladesign::Point point = m_manager->relation(1)->next(basePoint2, y);
			qreal height = m_manager->shapes()->heightForWidth(1.0);
			point.paint(painter, m_manager->shapes()->shape(), QRectF(-0.5, -height / 2.0, 1.0, height));
		}
	}
	painter->restore();
}

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

#include "onscreenanimator.h"
#include "onscreenwidget.h"

#include <QGraphicsView>

Palapeli::OnScreenAnimator::OnScreenAnimator(Palapeli::OnScreenWidget* widget)
	: QTimeLine(500)
	, m_widget(widget)
	, m_direction(NoDirection)
{
	setFrameRange(0, 15); //that is 30 fps which should be sufficient
	connect(this, SIGNAL(valueChanged(qreal)), this, SLOT(changeValue(qreal)));
}

Palapeli::OnScreenAnimator::Direction Palapeli::OnScreenAnimator::direction() const
{
	return m_direction;
}

void Palapeli::OnScreenAnimator::changeValue(qreal value)
{
	//TODO: LTR/RTL
	if (!m_widget)
		return;
	//find base point of animation, and size of widget
	QList<QGraphicsView*> views = m_widget->scene()->views();
	if (views.isEmpty())
		return;
	const QPointF basePoint = views[0]->mapToScene(0, 0);
	const QSizeF widgetSize = m_widget->boundingRect().size();
	//find start and end point of animation
	QPointF startPoint, endPoint;
	switch (m_direction)
	{
		case NoDirection:
			startPoint = endPoint = basePoint;
			break;
		case ShowDirection:
			startPoint = QPointF(basePoint.x() - widgetSize.width(), basePoint.y() - widgetSize.height());
			endPoint = basePoint;
			break;
		case HideDirection:
			startPoint = basePoint;
			endPoint = QPointF(basePoint.x() - widgetSize.width(), basePoint.y() - widgetSize.height());
			break;
	}
	m_widget->setPos(startPoint + value * (endPoint - startPoint));
}

void Palapeli::OnScreenAnimator::start(Palapeli::OnScreenAnimator::Direction direction)
{
	m_direction = direction;
	QTimeLine::start();
	changeValue(0); //move widget to start point
	m_widget->show(); //make sure the widget is visible during the animation
}

void Palapeli::OnScreenAnimator::animationEnd()
{
	m_direction = NoDirection; //ensures that m_direction != NoDirection only during animations
}

#include "onscreenanimator.moc"

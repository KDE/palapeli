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

#include <QApplication>
#include <QTimeLine>

const int Duration = 300;

Palapeli::OnScreenAnimator::OnScreenAnimator(Palapeli::OnScreenWidget* widget)
	: m_timeLine(new QTimeLine(Duration))
	, m_widget(widget)
	, m_direction(NoDirection)
{
	m_timeLine->setFrameRange(0, 10); //that is ~30 fps which should be sufficient
}

Palapeli::OnScreenAnimator::~OnScreenAnimator()
{
	delete m_timeLine;
}

Palapeli::OnScreenAnimator::Direction Palapeli::OnScreenAnimator::direction() const
{
	return m_direction;
}

int Palapeli::OnScreenAnimator::duration() const
{
	return 2 * Duration;
}

void Palapeli::OnScreenAnimator::changeValue1(qreal value)
{
	if (m_direction == ShowDirection)
		changeValuePosition(value);
	else //m_direction == HideDirection
		changeValueOpacity(value);
}

void Palapeli::OnScreenAnimator::changeValue2(qreal value)
{
	if (m_direction == ShowDirection)
		changeValueOpacity(value);
	else //m_direction == HideDirection
		changeValuePosition(value);
}

void Palapeli::OnScreenAnimator::changeValuePosition(qreal value)
{
	if (!m_widget)
		return;
	//find size of widget
	const QSizeF widgetSize = m_widget->boundingRect().size();
	//find start and end point of animation
	QPointF startPoint, endPoint;
	switch (m_direction)
	{
		case NoDirection:
			if (QApplication::isLeftToRight())
				startPoint = endPoint = QPointF(0.0, 0.0);
			else
				startPoint = endPoint = QPointF(-widgetSize.width(), 0.0);
			break;
		case ShowDirection:
			if (QApplication::isLeftToRight())
			{
				startPoint = QPointF(-widgetSize.width(), -widgetSize.height());
				endPoint = QPointF(0.0, 0.0);
			}
			else
			{
				startPoint = QPointF(0.0, -widgetSize.height());
				endPoint = QPointF(-widgetSize.width(), 0.0);
			}
			break;
		case HideDirection:
			if (QApplication::isLeftToRight())
			{
				startPoint = QPointF(0.0, 0.0);
				endPoint = QPointF(-widgetSize.width(), -widgetSize.height());
			}
			else
			{
				startPoint = QPointF(-widgetSize.width(), 0.0);
				endPoint = QPointF(0.0, -widgetSize.height());
			}
			break;
	}
	m_widget->setPos(startPoint + value * (endPoint - startPoint));
}

void Palapeli::OnScreenAnimator::changeValueOpacity(qreal value)
{
	if (m_direction == ShowDirection)
		m_widget->setForegroundOpacity(value);
	else //HideDirection
		m_widget->setForegroundOpacity(1.0 - value);
}

void Palapeli::OnScreenAnimator::start(Palapeli::OnScreenAnimator::Direction direction)
{
	m_direction = direction;
	m_timeLine->setCurveShape(QTimeLine::EaseInCurve);
	m_timeLine->start();
	//change connections
	m_timeLine->disconnect();
	connect(m_timeLine, SIGNAL(valueChanged(qreal)), this, SLOT(changeValue1(qreal)));
	connect(m_timeLine, SIGNAL(finished()), this, SLOT(animationEnd1()));
	//ensure correct start values
	changeValue1(0.0);
	changeValue2(0.0);
	//make sure the widget is visible during the animation
	m_widget->show();
}

void Palapeli::OnScreenAnimator::animationEnd1()
{
	m_timeLine->setCurveShape(QTimeLine::EaseOutCurve);
	m_timeLine->start();
	//change connections
	m_timeLine->disconnect();
	connect(m_timeLine, SIGNAL(valueChanged(qreal)), this, SLOT(changeValue2(qreal)));
	connect(m_timeLine, SIGNAL(finished()), this, SLOT(animationEnd2()));
	//ensure correct start values
	changeValue1(1.0);
	changeValue2(0.0);
}

void Palapeli::OnScreenAnimator::animationEnd2()
{
	m_direction = NoDirection; //ensures that m_direction != NoDirection only during animations
	emit finished();
}

#include "onscreenanimator.moc"

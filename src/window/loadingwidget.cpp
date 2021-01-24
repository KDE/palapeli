/*
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "loadingwidget.h"

#include <cmath>
#include <QPainter>
#include <QRadialGradient>
#include <QTimer>

Palapeli::LoadingWidget::LoadingWidget(QWidget* parent)
	: QWidget(parent)
	, m_updateTimer(new QTimer(this))
	, m_angleDegrees(0)
{
	setMinimumSize(QSize(64, 64));
	m_updateTimer->setInterval(30);
	connect(m_updateTimer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
}

void Palapeli::LoadingWidget::showEvent(QShowEvent* event)
{
	Q_UNUSED(event)
	m_updateTimer->start();
}

void Palapeli::LoadingWidget::hideEvent(QHideEvent* event)
{
	Q_UNUSED(event)
	m_updateTimer->stop();
}

void Palapeli::LoadingWidget::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event)
	m_angleDegrees = (m_angleDegrees + 14) % 360;
	const qreal pointerAngle = 2 * M_PI * m_angleDegrees / 360.0;
	const QPointF pointerDirection(cos(pointerAngle), sin(pointerAngle));

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	const QColor baseColor = painter.pen().color();
	const QColor baseColorTransparent(baseColor.red(), baseColor.green(), baseColor.blue(), 0);
	painter.setPen(Qt::NoPen);

	//metrics
	const QPoint center = rect().center();
	const qreal distance = 32;
	for (int i = 0; i < 8; ++i)
	{
		//determine position of circle
		const qreal angle = 0.25 * M_PI * i;
		const QPointF thisDirection(cos(angle), sin(angle));
		const QPointF thisCenter = center + distance * thisDirection;
		//determine size of circle
		const qreal scalarProd = thisDirection.x() * pointerDirection.x() + thisDirection.y() * pointerDirection.y();
		const qreal thisRadius = 10 + 5 * scalarProd;
		QSizeF thisSize(2 * thisRadius, 2 * thisRadius);
		//create gradient for circle
		QRadialGradient gradient(thisCenter, thisRadius);
		gradient.setColorAt(0, baseColor);
		gradient.setColorAt(0.4, baseColor);
		gradient.setColorAt(0.6, baseColorTransparent);
		painter.setBrush(gradient);
		//draw circle
		QRectF thisRect(QPointF(), thisSize);
		thisRect.moveCenter(thisCenter);
		painter.drawEllipse(thisRect);
	}
}

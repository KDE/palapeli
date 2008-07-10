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

#include "view.h"
#include "algebra.h"
#include "manager.h"
#include "mouseinteractor.h"
#include "objview.h"
#include "points.h"
#include "propmodel.h"
#include "relation.h"

#include <QMouseEvent>
#include <QPainter>

//TODO: Include in Manager a function for unified access to all interactors.

Paladesign::View::View(Paladesign::Manager* manager, QWidget* parent)
	: QWidget(parent)
	, m_manager(manager)
{
	setMouseTracking(true);
	setFocusPolicy(Qt::StrongFocus);
	
}

QPointF Paladesign::View::widgetToScene(const QPoint& point)
{
	const qreal x = point.x() - m_offset.x();
	const qreal y = -(point.y() - m_offset.y()); //the Y axis has a negative scaling factor because one normally expects the Y axis to be pointing upwards
	return QPointF(x, y) / m_scalingFactor;
}

void Paladesign::View::mousePressEvent(QMouseEvent* event)
{
	const QPointF point = widgetToScene(event->pos());
	if (event->button() != Qt::LeftButton)
		return;
	//select hovered interactor (if there is one)
	Paladesign::MouseInteractor* selectedInteractor = 0;
	//check Points
	Paladesign::MouseInteractor* interactor = m_manager->points();
	const bool hovered = interactor->hovered();
	interactor->setMousePosition(point);
	interactor->setSelected(hovered);
	if (hovered)
	{
		interactor->setClicked(interactor->clickAreaContains(point));
		selectedInteractor = interactor;
	}
	else
		interactor->setClicked(false);
	//check Relations
	for (int i = 0; i < m_manager->relationCount(); ++i)
	{
		interactor = m_manager->relation(i);
		const bool hovered = interactor->hovered();
		interactor->setMousePosition(point);
		interactor->setSelected(hovered);
		if (hovered)
		{
			interactor->setClicked(interactor->clickAreaContains(point));
			selectedInteractor = interactor;
		}
		else
			interactor->setClicked(false);
	}
	//the case selectedInteractor == 0 (i.e. nothing hovered when clicking -> everything unselected) is explicitly allowed
	m_manager->propertyModel()->setObject(selectedInteractor, SIGNAL(interactorChanged()));
	m_manager->objectView()->changeSelectedItem(selectedInteractor);
}

void Paladesign::View::mouseMoveEvent(QMouseEvent* event)
{
	setFocus(Qt::MouseFocusReason);
	const QPointF point = widgetToScene(event->pos());
	//if dragging is in progress, simply propagate event
	if (event->buttons() != Qt::NoButton)
	{
		for (int i = 0; i < m_manager->relationCount(); ++i)
			m_manager->relation(i)->setMousePosition(point);
		return;
	}
	//no dragging in progress - find hovered object
	bool foundHoveredObject = false; //select only one interactor at once
	//check Points interactor
	Paladesign::MouseInteractor* interactor = m_manager->points();
	if (interactor->hoverAreaContains(point))
	{
		foundHoveredObject = true;
		interactor->setHovered(true);
	}
	else
		interactor->setHovered(false);
	interactor->setClicked(false);
	//check Relation interactors
	for (int i = 0; i < m_manager->relationCount(); ++i)
	{
		interactor = m_manager->relation(i);
		const bool hovered = interactor->hoverAreaContains(point);
		if (!foundHoveredObject && hovered)
		{
			foundHoveredObject = true;
			interactor->setHovered(true);
		}
		else
			interactor->setHovered(false);
		interactor->setClicked(false);
	}
}

void Paladesign::View::mouseReleaseEvent(QMouseEvent* event)
{
	const QPointF point = widgetToScene(event->pos());
	if (event->button() != Qt::LeftButton)
		return;
	//propagate event to Points and Relations
	Paladesign::MouseInteractor* interactor = m_manager->points();
	interactor->setMousePosition(point);
	interactor->setClicked(false);
	for (int i = 0; i < m_manager->relationCount(); ++i)
	{
		interactor = m_manager->relation(i);
		interactor->setMousePosition(point);
		interactor->setClicked(false);
	}
}

void Paladesign::View::leaveEvent(QEvent*)
{
	//propagate event to Points and Relations - remove hover state if not clicked (i.e. dragging)
	Paladesign::MouseInteractor* interactor = m_manager->points();
	if (!interactor->clicked())
		interactor->setHovered(false);
	for (int i = 0; i < m_manager->relationCount(); ++i)
	{
		interactor = m_manager->relation(i);
		if (!interactor->clicked())
			interactor->setHovered(false);
	}
}

void Paladesign::View::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::HighQualityAntialiasing);
	painter.translate(m_offset);
	painter.scale(m_scalingFactor, -m_scalingFactor); //the Y axis has a negative scaling factor because one normally expects the Y axis to be pointing upwards

	QPointF clipTopLeft = widgetToScene(QPoint(0.0, 0.0));
	QPointF clipBottomRight = widgetToScene(QPoint(width(), height()));
	QRectF clipRect(clipTopLeft, clipBottomRight);
	m_manager->points()->paint(&painter, clipRect);
}

void Paladesign::View::resizeEvent(QResizeEvent*)
{
	m_offset = QPoint(width() / 2, height() / 2);
	static const qreal edgeLength = 2 * Paladesign::Points::ItemRange + 1; //visible coordinates range between -10 and 10 on each axis; plus 0.5 units padding on each side
	m_scalingFactor = qMin((qreal) width(), (qreal) height()) / edgeLength;
	update();
}

void Paladesign::View::select(QObject* object)
{
	Paladesign::MouseInteractor* selected = qobject_cast<Paladesign::MouseInteractor*>(object);
	if (selected != 0)
	{
		if (selected->selected())
			//this interactor is already selected - nothing to do
			return;
	}
	//apply selection to interactors (Points and relations)
	Paladesign::MouseInteractor* interactor = m_manager->points();
	interactor->setSelected(interactor == object);
	for (int i = 0; i < m_manager->relationCount(); ++i)
	{
		interactor = m_manager->relation(i);
		interactor->setSelected(interactor == object);
	}
	m_manager->propertyModel()->setObject(object, SIGNAL(interactorChanged()));
	update();
}

#include "view.moc"

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
	//click on selected interactor (if there is one)
	Paladesign::MouseInteractor* selectedInteractor = 0;
	for (int i = 0; i < m_manager->relationCount(); ++i)
	{
		Paladesign::MouseInteractor* interactor = m_manager->relation(i);
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
	//no dragging in progress
	if (Paladesign::Algebra::vectorLength(point) > Paladesign::Points::MaximumSelectionDistance)
	{
		//mouse out of forbidden area in the center - find selected interactor
		bool foundHoveredObject = false; //select only one interactor at once
		for (int i = 0; i < m_manager->relationCount(); ++i)
		{
			Paladesign::MouseInteractor* interactor = m_manager->relation(i);
			const bool hovered = interactor->hoverAreaContains(point);
			if (foundHoveredObject || !hovered)
				interactor->setHovered(false);
			else
			{
				foundHoveredObject = true;
				interactor->setHovered(true);
			}
			interactor->setClicked(false);
		}
	}
	else
	{
		//mouse in forbidden area (where many interactors overlap) - do not hover anything
		for (int i = 0; i < m_manager->relationCount(); ++i)
			m_manager->relation(i)->setHovered(false);
	}
}

void Paladesign::View::mouseReleaseEvent(QMouseEvent* event)
{
	const QPointF point = widgetToScene(event->pos());
	if (event->button() != Qt::LeftButton)
		return;
	for (int i = 0; i < m_manager->relationCount(); ++i)
	{
		Paladesign::MouseInteractor* interactor = m_manager->relation(i);
		interactor->setMousePosition(point);
		interactor->setClicked(false);
	}
}

void Paladesign::View::leaveEvent(QEvent*)
{
	for (int i = 0; i < m_manager->relationCount(); ++i)
	{
		//remove hover state if not clicked (i.e. dragging)
		Paladesign::MouseInteractor* interactor = m_manager->relation(i);
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
	for (int i = 0; i < m_manager->relationCount(); ++i)
	{
		Paladesign::MouseInteractor* interactor = m_manager->relation(i);
		interactor->setSelected(interactor == selected);
	}
	m_manager->propertyModel()->setObject(selected, SIGNAL(interactorChanged()));
	update();
}

#include "view.moc"

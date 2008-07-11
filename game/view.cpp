/***************************************************************************
 *   Copyright (C) 2008 Felix Lemke <lemke.felix@ages-skripte.org>
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
#include "manager.h"
#include "settings.h"

#include <QGraphicsScene>
#ifdef PALAPELI_WITH_OPENGL
#	include <QGLWidget>
#endif
#include <QScrollBar>
#include <QWheelEvent>

Palapeli::View::View(QWidget* parent)
	: QGraphicsView(parent)
	, m_scene(new QGraphicsScene)
{
	connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), ppMgr(), SLOT(updateMinimap()));
	connect(verticalScrollBar(), SIGNAL(valueChanged(int)), ppMgr(), SLOT(updateMinimap()));
	setScene(m_scene);
	//load settings
	Settings::self()->readConfig();
	setAntialiasing(Settings::antialiasing(), true);
	setHardwareAccelerated(Settings::hardwareAccel(), true);
}

void Palapeli::View::wheelEvent(QWheelEvent* event)
{
	qreal delta = event->delta();
	if (event->modifiers() & Qt::ControlModifier)
	{
		//control + mouse wheel - zoom viewport in/out
		static const qreal deltaAdaptationFactor = 600.0;
		qreal scalingFactor = 1.0 + qAbs(delta) / deltaAdaptationFactor;
		if (delta < 0) //zoom out
		{
			scalingFactor = 1.0 / scalingFactor;
			if (scalingFactor <= 0.01)
				scalingFactor = 0.01;
		}
		scale(scalingFactor, scalingFactor);
		ppMgr()->updateMinimap();
	}
	else if ((event->modifiers() & Qt::ShiftModifier) || event->orientation() == Qt::Horizontal)
		//shift + mouse wheel - move the viewport left/right by adjusting the slider
		horizontalScrollBar()->triggerAction(delta < 0 ? QAbstractSlider::SliderSingleStepAdd : QAbstractSlider::SliderSingleStepSub);
	else
		//just the mouse wheel - move the viewport up/down by adjusting the slider
		verticalScrollBar()->triggerAction(delta < 0 ? QAbstractSlider::SliderSingleStepAdd : QAbstractSlider::SliderSingleStepSub);
}

void Palapeli::View::setAntialiasing(bool useAntialiasing, bool forceApplication)
{
	if (Settings::antialiasing() == useAntialiasing && !forceApplication) //nothing to do (and application not forced)
		return;
	Settings::setAntialiasing(useAntialiasing);
	Settings::self()->writeConfig();
	//apply settings
	setRenderHint(QPainter::Antialiasing, useAntialiasing);
	setRenderHint(QPainter::HighQualityAntialiasing, useAntialiasing);
}

void Palapeli::View::setHardwareAccelerated(bool useHardware, bool forceApplication)
{
	if (Settings::hardwareAccel() == useHardware && !forceApplication) //nothing to do (and application not forced)
		return;
	Settings::setHardwareAccel(useHardware);
	Settings::self()->writeConfig();
	//apply settings
#ifdef PALAPELI_WITH_OPENGL
	viewport()->deleteLater();
	setViewport(useHardware ? new QGLWidget : new QWidget);
#endif
}

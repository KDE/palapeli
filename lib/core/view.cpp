/***************************************************************************
 *   Copyright (C) 2008 Felix Lemke <lemke.felix@ages-skripte.org>
 *   Copyright (C) 2008-2009 Stefan Majewsky <majewsky@gmx.net>
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
#include "viewmenu.h"
#include "settings.h"

#include <QBrush>
#include <QContextMenuEvent>
#include <QFileInfo>
#include <QGraphicsItem>
#include <QGraphicsScene>
#ifdef PALAPELI_WITH_OPENGL
#	include <QGLWidget>
#endif
#include <QPainter>
#include <QScrollBar>
#include <QSignalMapper>
#include <QSvgRenderer>
#include <QWheelEvent>
#include <KAction>
#include <KLocalizedString>
#include <KMenu>
#include <KStandardDirs>

Palapeli::View::View(QWidget* parent)
	: QGraphicsView(parent)
	, m_menu(new Palapeli::ViewMenu(this))
	, m_scene(new QGraphicsScene)
{
	//initialize view and scene
	setScene(m_scene);
	m_scene->setSceneRect(QRectF(-1, -1, 2, 2)); //the exact values are not important as long as there is some specific scene rect
	updateBackground(m_menu->currentBackground());
	//load settings
	Settings::self()->readConfig();
	setAntialiasing(Settings::antialiasing(), true);
	setHardwareAccelerated(Settings::hardwareAccel(), true);
	//report viewport moves
	connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SIGNAL(viewportMoved()));
	connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SIGNAL(viewportMoved()));
	//react on background changes
	connect(m_menu, SIGNAL(backgroundSelected(const QBrush&)), this, SLOT(updateBackground(const QBrush&)));
}

Palapeli::View::~View()
{
	setHardwareAccelerated(false); //prevent a crash that occurs if a QGLWidget viewport is active while deleting the View
}

void Palapeli::View::updateBackground(const QBrush& brush)
{
	m_scene->setBackgroundBrush(brush);
}

void Palapeli::View::contextMenuEvent(QContextMenuEvent* event)
{
	Q_UNUSED(event)
	m_menu->showAtCursorPosition();
}

void Palapeli::View::resizeEvent(QResizeEvent* event)
{
	Q_UNUSED(event)
	emit viewportMoved();
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
		scale(scalingFactor);
	}
	else if ((event->modifiers() & Qt::ShiftModifier) || event->orientation() == Qt::Horizontal)
		//shift + mouse wheel - move the viewport left/right by adjusting the slider
		horizontalScrollBar()->triggerAction(delta < 0 ? QAbstractSlider::SliderSingleStepAdd : QAbstractSlider::SliderSingleStepSub);
	else
		//just the mouse wheel - move the viewport up/down by adjusting the slider
		verticalScrollBar()->triggerAction(delta < 0 ? QAbstractSlider::SliderSingleStepAdd : QAbstractSlider::SliderSingleStepSub);
}

void Palapeli::View::scale(qreal scalingFactor)
{
	if (scalingFactor == 0)
		fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
	else
		QGraphicsView::scale(scalingFactor, scalingFactor);
	emit viewportScaled();
}

void Palapeli::View::setAntialiasing(bool useAntialiasing, bool forceApplication)
{
	if (Settings::antialiasing() == useAntialiasing && !forceApplication) //nothing to do (and application not forced)
		return;
	Settings::setAntialiasing(useAntialiasing);
	//apply settings
	setRenderHint(QPainter::Antialiasing, useAntialiasing);
	setRenderHint(QPainter::HighQualityAntialiasing, useAntialiasing);
}

void Palapeli::View::setHardwareAccelerated(bool useHardware, bool forceApplication)
{
	if (Settings::hardwareAccel() == useHardware && !forceApplication) //nothing to do (and application not forced)
		return;
	Settings::setHardwareAccel(useHardware);
	//apply settings
#ifdef PALAPELI_WITH_OPENGL
	viewport()->deleteLater();
	setViewport(useHardware ? new QGLWidget : new QWidget);
#endif
}

Palapeli::ViewMenu* Palapeli::View::menu() const
{
	return m_menu;
}

QGraphicsScene* Palapeli::View::realScene() const
{
	return m_scene;
}

void Palapeli::View::useScene(bool useScene)
{
	setScene(useScene ? m_scene : 0);
	if (useScene)
	{
		scale(0);
		emit viewportMoved();
	}
}

void Palapeli::View::moveToTop(QGraphicsItem* item) const
{
	static int currentZValue = 0;
	item->setZValue(++currentZValue);
}

#include "view.moc"

/***************************************************************************
 *   Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
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
#include "scene.h"
#include "viewmenu.h"

#include <QWheelEvent>

Palapeli::View::View()
	: m_scene(new Palapeli::Scene(this))
	, m_menu(new Palapeli::ViewMenu(m_scene))
	, m_zoomLevel(0) //set to invalid level while scene has not specified its rect
{
	//initialize viewport and scene
	setDragMode(QGraphicsView::ScrollHandDrag);
	setScene(m_scene);
	connect(m_scene, SIGNAL(sceneRectChanged(const QRectF&)), this, SLOT(sceneRectChanged(const QRectF&)));
	//initialize menu
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), m_menu, SLOT(showAtCursorPosition()));
}

Palapeli::Scene* Palapeli::View::scene() const
{
	return m_scene;
}

void Palapeli::View::resizeEvent(QResizeEvent* event)
{
	QGraphicsView::resizeEvent(event);
	//do not always react on resize events with viewport matrix changes, but make the scene appear bigger if the viewport grows much to big
	restrictViewportToSceneRect();
}

void Palapeli::View::wheelEvent(QWheelEvent* event)
{
	zoomBy(event->delta());
}

void Palapeli::View::zoomBy(int delta)
{
	static const qreal deltaAdaptationFactor = 600.0;
	qreal scalingFactor = 1.0 + qAbs(delta) / deltaAdaptationFactor;
	if (delta < 0) //zoom out
	{
		scalingFactor = 1.0 / scalingFactor;
		if (scalingFactor <= 0.01)
			scalingFactor = 0.01;
	}
	zoomTo(m_zoomLevel * scalingFactor);
}

void Palapeli::View::zoomTo(qreal level)
{
	//validate/normalize input
	if (level <= 0)
		return;
	if (level >= 10)
		level = 10;
	if (level < 1)
		level = 1;
	//skip unimportant requests
	if (qFuzzyCompare(level, m_zoomLevel))
		return;
	//scale to this level
	qreal scalingFactor = level / m_zoomLevel;
	scale(scalingFactor, scalingFactor);
	m_zoomLevel = level;
	//extra sanity check
	restrictViewportToSceneRect();
	//report changes
	emit zoomLevelChanged(m_zoomLevel);
}

void Palapeli::View::zoomIn()
{
	zoomBy(120);
}

void Palapeli::View::zoomOut()
{
	zoomBy(-120);
}

void Palapeli::View::sceneRectChanged(const QRectF& sceneRect)
{
	m_zoomLevel = 1.0;
	fitInView(sceneRect, Qt::KeepAspectRatio);
	setSceneRect(sceneRect);
	//report changes
	emit zoomLevelChanged(m_zoomLevel);
}

void Palapeli::View::restrictViewportToSceneRect()
{
	//Rationale of this method: Adjust the viewport matrix in such a way that the scene rect is never displayed smaller than it would be after a call to fitInView(sceneRect, Qt::KeepAspectRatio).
	//do not allow viewport to grow bigger than the scene rect
	const QRectF sr = m_scene->sceneRect();
	const QRectF vr = mapToScene(contentsRect()).boundingRect();
	if (sr.isEmpty() && vr.isEmpty())
		return;
	else if (vr.width() > sr.width() && vr.height() > sr.height())
	{
		m_zoomLevel = 1.0;
		fitInView(sr, Qt::KeepAspectRatio);
		//report changes
		emit zoomLevelChanged(m_zoomLevel);
	}
}

#include "view.moc"

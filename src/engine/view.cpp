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

void Palapeli::View::wheelEvent(QWheelEvent* event)
{
	//zoom viewport in/out
	const qreal delta = event->delta();
	static const qreal deltaAdaptationFactor = 600.0;
	qreal scalingFactor = 1.0 + qAbs(delta) / deltaAdaptationFactor;
	if (delta < 0) //zoom out
	{
		scalingFactor = 1.0 / scalingFactor;
		if (scalingFactor <= 0.01)
			scalingFactor = 0.01;
	}
	scale(scalingFactor, scalingFactor);
}

void Palapeli::View::sceneRectChanged(const QRectF& sceneRect)
{
	fitInView(sceneRect);
}

#include "view.moc"

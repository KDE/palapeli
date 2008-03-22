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
#include "scene.h"

#include <QScrollBar>
#include <QWheelEvent>

Palapeli::View::View(QWidget* parent)
	: QGraphicsView(parent)
	, m_scene(0)
{
	//setDragMode(QGraphicsView::ScrollHandDrag);
}

Palapeli::View::~View()
{
	delete m_scene;
}

void Palapeli::View::startGame(int sceneWidth, int sceneHeight, const QString &fileName, int xPieces, int yPieces)
{
	delete m_scene;
	m_scene = new Palapeli::Scene(sceneWidth, sceneHeight);
	m_scene->loadImage(fileName, xPieces, yPieces);
	setScene(m_scene);
}

void Palapeli::View::wheelEvent(QWheelEvent* event)
{
	qreal delta = event->delta();
	if (event->modifiers() & Qt::ControlModifier)
	{
		//control + mouse wheel - zoom viewport in/out
		const qreal deltaAdaptationFactor = 600.0;
		const qreal scaleBy = qAbs(delta) / deltaAdaptationFactor;
		if (delta < 0)
			scale(1 - scaleBy, 1 - scaleBy);
		else
			scale(1 + scaleBy, 1 + scaleBy);
	}
	else if (event->modifiers() & Qt::ShiftModifier)
	{
		//shift + mouse wheel - move the viewport left/right by adjusting the slider
		horizontalScrollBar()->triggerAction(delta < 0 ? QAbstractSlider::SliderSingleStepAdd : QAbstractSlider::SliderSingleStepSub);
	}
	else
	{
		//just the mouse wheel - move the viewport up/down by adjusting the slider
		verticalScrollBar()->triggerAction(delta < 0 ? QAbstractSlider::SliderSingleStepAdd : QAbstractSlider::SliderSingleStepSub);
	}
}

#include "view.moc"

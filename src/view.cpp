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

#include <QImage>
#include <QScrollBar>
#include <QWheelEvent>
#include <KLocalizedString>

Palapeli::View::View(QWidget* parent)
	: QGraphicsView(parent)
	, m_scene(0)
{
	setWindowTitle(i18nc("The application's name", "Palapeli"));
	resize(400, 400);
	connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SIGNAL(viewportMoved()));
	connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SIGNAL(viewportMoved()));
}

Palapeli::View::~View()
{
	delete m_scene;
}

Palapeli::Scene* Palapeli::View::puzzleScene() const
{
	return m_scene;
}

void Palapeli::View::startGame(int sceneWidth, int sceneHeight, const QString &fileName, int xPieces, int yPieces)
{
	delete m_scene;
	QImage image(fileName);
	m_scene = new Palapeli::Scene(
		sceneWidth == -1 ? 2 * image.width() : sceneWidth,
		sceneHeight == -1 ? 2 * image.height() : sceneHeight
	);
	m_scene->loadImage(image, xPieces, yPieces);
	setScene(m_scene);
}

void Palapeli::View::wheelEvent(QWheelEvent* event)
{
	qreal delta = event->delta();
	if (event->modifiers() & Qt::ControlModifier)
	{
		//control + mouse wheel - zoom viewport in/out
		const qreal deltaAdaptationFactor = 600.0;
		qreal scalingFactor = 1 + delta / deltaAdaptationFactor;
		if (scalingFactor <= 0.01)
			scalingFactor = 0.01;
		scale(scalingFactor, scalingFactor);
		emit viewportMoved();
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

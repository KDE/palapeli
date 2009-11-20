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
#include "constraintvisualizer.h"
#include "scene.h"
#include "settings.h"
#include "texturehelper.h"

#include <cmath>
#include <QPropertyAnimation>
#include <QScrollBar>
#include <QWheelEvent>
#include <KLocalizedString>
#include <KMessageBox>

const int Palapeli::View::MinimumZoomLevel = 0;
const int Palapeli::View::MaximumZoomLevel = 200;

Palapeli::View::View()
	: m_scene(new Palapeli::Scene(this))
	, m_constraintVisualizer(0) //cannot be initialized before scene has been set
	, m_txHelper(new Palapeli::TextureHelper(m_scene))
	, m_zoomLevel(100)
{
	if (Settings::leftMouseButtonMovesView())
		setDragMode(QGraphicsView::ScrollHandDrag);
	setResizeAnchor(QGraphicsView::AnchorUnderMouse);
	setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	setScene(m_scene);
	m_constraintVisualizer = new Palapeli::ConstraintVisualizer(this);
	connect(m_scene, SIGNAL(constrainedChanged(bool)), m_constraintVisualizer, SLOT(setActive(bool)));
	connect(m_scene, SIGNAL(puzzleStarted()), this, SLOT(puzzleStarted()));
	connect(m_scene, SIGNAL(victoryAnimationFinished()), this, SLOT(startVictoryAnimation()));
}

Palapeli::View::~View()
{
	delete m_txHelper;
}

Palapeli::Scene* Palapeli::View::scene() const
{
	return m_scene;
}

Palapeli::TextureHelper* Palapeli::View::textureHelper() const
{
	return m_txHelper;
}

QRectF Palapeli::View::viewportRect() const
{
	return mapToScene(viewport()->rect()).boundingRect();
}

void Palapeli::View::setViewportRect(const QRectF& viewportRect)
{
	//NOTE: Do never ever use this except for the victory animation, or stuff will break!!!
	fitInView(viewportRect, Qt::KeepAspectRatio);
}

void Palapeli::View::mousePressEvent(QMouseEvent* event)
{
	QGraphicsView::mousePressEvent(event);
	if (event->button() == Qt::RightButton)
		m_dragPrevPos = event->globalPos();
}

void Palapeli::View::mouseMoveEvent(QMouseEvent* event)
{
	QGraphicsView::mouseMoveEvent(event);
	if (event->buttons() & Qt::RightButton)
	{
		//move viewport
		const QPointF delta = event->globalPos() - m_dragPrevPos;
		horizontalScrollBar()->setValue(horizontalScrollBar()->value() + (isRightToLeft() ? delta.x() : -delta.x()));
		verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
		//store mouse position for next event
		m_dragPrevPos = event->globalPos();
	}
}

void Palapeli::View::wheelEvent(QWheelEvent* event)
{
	zoomBy(event->delta());
}

void Palapeli::View::zoomBy(int delta)
{
	zoomTo(m_zoomLevel + delta / 10);
}

void Palapeli::View::zoomTo(int level)
{
	//validate/normalize input
	level = qBound(MinimumZoomLevel, level, MaximumZoomLevel);
	//skip unimportant requests
	if (level == m_zoomLevel)
		return;
	//create a new transform
	const QPointF center = mapToScene(rect().center());
	const qreal scalingFactor = pow(2, (level - 100) / 30.0);
	QTransform t;
	t.translate(center.x(), center.y());
	t.scale(scalingFactor, scalingFactor);
	setTransform(t);
	//save and report changes
	m_zoomLevel = level;
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

void Palapeli::View::puzzleStarted()
{
	resetTransform();
	//scale viewport to show the whole puzzle table
	const QRectF sr = sceneRect();
	const QRectF vr = mapToScene(viewport()->rect()).boundingRect();
	const qreal scalingFactor = 0.9 * qMin(vr.width() / sr.width(), vr.height() / sr.height()); //factor 0.9 avoids that scene rect touches viewport bounds (which does not look nice)
	const int level = 100 + (int)(30.0 * (log(scalingFactor) / log(2.0)));
	zoomTo(level);
	centerOn(sr.center());
	emit zoomAdjustable(true);
	//explain autosaving
	KMessageBox::information(window(), i18n("Your progress is saved automatically while you play."), i18nc("used as caption for a dialog that explains the autosave feature", "Automatic saving"), QLatin1String("autosave-introduction"));
}

void Palapeli::View::startVictoryAnimation()
{
	//move viewport to show the complete puzzle
	QPropertyAnimation* animation = new QPropertyAnimation(this, "viewportRect", this);
	animation->setEndValue(m_scene->partsBoundingRect());
	animation->setDuration(1000);
	animation->start(QAbstractAnimation::DeleteWhenStopped);
	emit zoomAdjustable(false);
}

#include "view.moc"

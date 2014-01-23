/***************************************************************************
 *   Copyright 2009, 2010 Stefan Majewsky <majewsky@gmx.net>
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
#include "interactormanager.h"
#include "scene.h"
#include "texturehelper.h"

#include <cmath>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QScrollBar>
#include <KLocalizedString>
#include <KMessageBox>

#include <QApplication> // IDW test.
#include <QDesktopWidget> // IDW test.
#include <QDebug> // IDW test.

const int Palapeli::View::MinimumZoomLevel = 0;
const int Palapeli::View::MaximumZoomLevel = 200;

Palapeli::View::View()
	: m_interactorManager(new Palapeli::InteractorManager(this))
	, m_scene(0)
	, m_zoomLevel(100)
	, m_closeUpLevel(0)
	, m_previousLevel(0)
{
	setFrameStyle(QFrame::NoFrame);
	setMouseTracking(true);
	setResizeAnchor(QGraphicsView::AnchorUnderMouse);
	setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	setScene(new Palapeli::Scene(this));
	connect(m_scene, SIGNAL(puzzleStarted()), this, SLOT(puzzleStarted()));
	connect(m_scene, SIGNAL(sceneRectChanged(QRectF)), this, SLOT(logSceneChange(QRectF))); // IDW test.
	qDebug() << "Initial size of Palapeli::View" << size();
}

// IDW test.
void Palapeli::View::logSceneChange(QRectF r)
{
	qDebug() << "View::logSceneChange" << r << "View size" << this->size();
}

Palapeli::InteractorManager* Palapeli::View::interactorManager() const
{
	return m_interactorManager;
}

Palapeli::Scene* Palapeli::View::scene() const
{
	return m_scene;
}

void Palapeli::View::setScene(Palapeli::Scene* scene)
{
	if (m_scene == scene)
		return;
	m_scene = scene;
	this->QGraphicsView::setScene(m_scene);
	m_interactorManager->updateScene();
	Palapeli::TextureHelper::instance()->addScene(m_scene);
	//reset zoom level (TODO: store viewport geometry in Scene)
	zoomTo(100);
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

void Palapeli::View::keyPressEvent(QKeyEvent* event)
{
	m_interactorManager->handleEvent(event);
	QGraphicsView::keyPressEvent(event);
}

void Palapeli::View::keyReleaseEvent(QKeyEvent* event)
{
	m_interactorManager->handleEvent(event);
	QGraphicsView::keyReleaseEvent(event);
}

void Palapeli::View::mouseMoveEvent(QMouseEvent* event)
{
	m_interactorManager->handleEvent(event);
	event->accept();
	//send a stripped QMouseEvent to base class to update resizeAnchor() etc.
	QMouseEvent modifiedEvent(event->type(),
		event->pos(), event->globalPos(),
		Qt::NoButton, Qt::NoButton, event->modifiers()
	);
	QGraphicsView::mouseMoveEvent(&modifiedEvent);
}

void Palapeli::View::mousePressEvent(QMouseEvent* event)
{
	m_interactorManager->handleEvent(event);
	event->accept();
}

void Palapeli::View::mouseReleaseEvent(QMouseEvent* event)
{
	m_interactorManager->handleEvent(event);
	event->accept();
}

void Palapeli::View::wheelEvent(QWheelEvent* event)
{
	m_interactorManager->handleEvent(event);
	//We do intentionally *not* propagate to QGV::wheelEvent.
}

void Palapeli::View::moveViewportBy(const QPointF& sceneDelta)
{
	horizontalScrollBar()->setValue(horizontalScrollBar()->value() + (isRightToLeft() ? sceneDelta.x() : -sceneDelta.x()));
	verticalScrollBar()->setValue(verticalScrollBar()->value() - sceneDelta.y());
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

// IDW TODO - Keyboard shortcuts for moving the view left, right, up or down by
//            one "frame" or "page".  Map to Arrow keys, PageUp and PageDown.
//            Use QAbstractScrollArea (inherited by QGraphicsView) to get the
//            QScrollBar objects (horizontal and vertical).  QAbstractSlider,
//            an ancestor of QScrollBar, contains position info, signals and
//            triggers for scroll bar moves (i.e. triggerAction(action type)).

void Palapeli::View::toggleCloseUp()
{
	// IDW TODO - Make sure the mouse pointer stays at the same point in the
	//            scene as we change views.  It tends to drift off if we are
	//            near the edge of the scene as we go to close-up or if we
	//            move the mouse-pointer or the view during close-up.
	if (! m_closeUpLevel) {
		m_closeUpLevel = calculateCloseUpLevel();
	}
	if (m_zoomLevel != m_closeUpLevel) {
		m_previousLevel = m_zoomLevel;
		zoomTo(m_closeUpLevel);
	}
	else if (m_previousLevel) {
		zoomTo(m_previousLevel);
	}
}

int Palapeli::View::calculateCloseUpLevel()
{
	// IDW TODO - Make this a Setting with default based on monitor pixels.
	// Get the size of the monitor on which this view resides (in pixels).
	const QRect monitor = QApplication::desktop()->screenGeometry(this);
	const int pixelsPerPiece = qMin(monitor.width(), monitor.height())/12;
	QSizeF size = scene()->pieceAreaSize();
	qreal  zoom  = pixelsPerPiece/qMin(size.rwidth(),size.rheight());
	// IDW TODO - zoom = zoom * setting;	// Default 1.0.
	qDebug() << "Screen" << QApplication::desktop()->screenGeometry(this);
	qDebug() << "pix" << pixelsPerPiece << size << "zoom" << zoom << "level"
		 << (100 + (int)(30.0 * (log(zoom) / log(2.0))));
	return (100 + (int)(30.0 * (log(zoom) / log(2.0))));
}

void Palapeli::View::puzzleStarted()
{
	// IDW TODO - An event-loop has to be allowed by GamePlay before this
	//            is called and we get ugly flashes on some puzzles. Should
	//            we hide the View during loading and re-show it here? Or
	//            should we keep the dancing balls going for longer?
	//
	//            NOTE: Scene::startPuzzle() is signalling puzzleStarted()
	//            after a break for an event-loop requested by GamePlay,
	//            but it is GamePlay invoking m_puzzleTable->reportProgress
	//            immediately that turns off the dancing balls.
	resetTransform();
	m_closeUpLevel = 0;
	m_previousLevel = 0;
	// IDW TODO - fitInView(viewport()->rect(), Qt::KeepAspectRatio);
	qDebug() << "scene" << sceneRect() << "viewport" << viewport()->rect() << "mapToScene" << mapToScene(viewport()->rect()) << "bound" << mapToScene(viewport()->rect()).boundingRect();
	//scale viewport to show the whole puzzle table
	const QRectF sr = sceneRect();
	const QRectF vr = mapToScene(viewport()->rect()).boundingRect();
	const qreal scalingFactor = /* IDW test. 0.9 * */ qMin(vr.width() / sr.width(), vr.height() / sr.height()); //factor 0.9 avoids that scene rect touches viewport bounds (which does not look nice)
	qDebug() << "width ratio" << vr.width() / sr.width() << "height ratio" << vr.height() / sr.height() << "scalingFactor" << scalingFactor;
	const int level = 100 + (int)(30.0 * (log(scalingFactor) / log(2.0)));
	zoomTo(level);
	centerOn(sr.center());
	emit zoomAdjustable(true);
	qDebug() << "puzzleStarted(): size of Palapeli::View -" << size();
	//explain autosaving
	KMessageBox::information(window(), i18n("Your progress is saved automatically while you play."), i18nc("used as caption for a dialog that explains the autosave feature", "Automatic saving"), QLatin1String("autosave-introduction"));
}

void Palapeli::View::startVictoryAnimation()
{
	//move viewport to show the complete puzzle
	QPropertyAnimation* animation = new QPropertyAnimation(this, "viewportRect", this);
	animation->setEndValue(m_scene->piecesBoundingRect());
	animation->setDuration(1000);
	animation->start(QAbstractAnimation::DeleteWhenStopped);
	emit zoomAdjustable(false);
}

#include "view.moc"

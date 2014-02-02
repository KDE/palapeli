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

#include <QApplication>
#include <QDesktopWidget>
#include <QDebug> // IDW test.

const int Palapeli::View::MinimumZoomLevel = 0;
const int Palapeli::View::MaximumZoomLevel = 200;

Palapeli::View::View()
	: m_interactorManager(new Palapeli::InteractorManager(this))
	, m_scene(0)
	, m_zoomLevel(MinimumZoomLevel)
	, m_closeUpLevel(MaximumZoomLevel)
	, m_distantLevel(MinimumZoomLevel)
	, m_isCloseUp(false)
	, m_dZoom(20.0)
	, m_minScale(0.01)
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
	// Draw empty, hidden scene: needed to get first load resized correctly.
	scene->addMargin(20.0, 10.0);
	// Set zoom level to middle of range.
	zoomTo((MaximumZoomLevel+MinimumZoomLevel)/2);
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
	// IDW TODO - BUG: If you zoom out as far as Palapeli will go, using the
	//            scroll-wheel, then go on scrolling, the view will zoom in
	//            and back out again momentarily.
	//
	//validate/normalize input
	level = qBound(MinimumZoomLevel, level, MaximumZoomLevel);
	//skip unimportant requests
	if (level == m_zoomLevel) {
		qDebug() << "ZOOM LEVEL UNCHANGED: View::zoomTo:" << level;
		return;
	}
	//create a new transform
	const QPointF center = mapToScene(rect().center());
	const qreal scalingFactor = m_minScale * pow(2, level/m_dZoom);
	qDebug() << "View::zoomTo: level" << level
		 << "scalingFactor" << scalingFactor;
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

// NOTE: We must have m_closeUpLevel >= (m_distantLevel + MinDiff) at all times.
const int MinDiff = 10;		// Minimum separation of the two zoom levels.

void Palapeli::View::toggleCloseUp()
{
	// IDW TODO - Make sure the mouse pointer stays at the same point in the
	//            scene as we change views.  It tends to drift off if we are
	//            near the edge of the scene as we go to close-up or if we
	//            move the mouse-pointer or the view during close-up.

	qDebug() << "View::toggleCloseUp()" << m_closeUpLevel << m_distantLevel
		 << "current" << m_zoomLevel << m_isCloseUp;
	m_isCloseUp = !m_isCloseUp;	// Switch to the other view.
	if (m_isCloseUp) {
		// Save distant level as we leave: in case it changed.
		m_distantLevel = (m_zoomLevel <= (m_closeUpLevel - MinDiff)) ?
					m_zoomLevel : m_closeUpLevel - MinDiff;
		zoomTo(m_closeUpLevel);
	}
	else {
		// Save close-up level as we leave: in case it changed.
		m_closeUpLevel = (m_zoomLevel >= (m_distantLevel + MinDiff)) ?
					m_zoomLevel : m_distantLevel + MinDiff;
		zoomTo(m_distantLevel);
	}
}

qreal Palapeli::View::calculateCloseUpScale()
{
	// Get the size of the monitor on which this view resides (in pixels).
	const QRect monitor = QApplication::desktop()->screenGeometry(this);
	const int pixelsPerPiece = qMin(monitor.width(), monitor.height())/12;
	QSizeF size = scene()->pieceAreaSize();
	qreal  scale  = pixelsPerPiece/qMin(size.rwidth(),size.rheight());
	return scale;
}

void Palapeli::View::puzzleStarted()
{
	// At this point the whole puzzle area has been scaled to fit the view.
	// Now adjust zooming and slider to range of distant and close-up views.
	//
	// Choose the lesser of the horizontal and vertical scaling factors.
	const qreal scalingFactor = qMin(transform().m11(), transform().m22());
	qDebug() << "scalingFactor" << scalingFactor;
	qreal closeUpScale = calculateCloseUpScale();
	if (closeUpScale < scalingFactor) {
		closeUpScale = scalingFactor;	// View is already large enough.
	}
	const qreal minScale = scalingFactor*0.75;
	const qreal maxScale = closeUpScale*2.0;
	const qreal range = log(maxScale/minScale)/log(2.0);
	const qreal dZoom = (MaximumZoomLevel - MinimumZoomLevel)/range;
	qDebug() << "minScale" << minScale << "maxScale" << maxScale
		 << "range" << range << "dZoom" << dZoom;
	m_dZoom = dZoom;
	m_minScale = minScale;

	const int level = qRound(dZoom*log(scalingFactor/minScale)/log(2.0));
	qDebug() << "INITIAL LEVEL" << level;

	// Set the toggling levels. If close-up is too small, adjust it.
	m_closeUpLevel = qRound(MaximumZoomLevel - MinimumZoomLevel - m_dZoom);
	m_distantLevel = level;
	m_closeUpLevel = (m_closeUpLevel >= (m_distantLevel + MinDiff)) ?
				m_closeUpLevel : m_distantLevel + MinDiff;
	m_isCloseUp = false;		// Start with the view zoomed out.

	// Don't readjust the zoom. Just set the slider pointer.
	m_zoomLevel = level;		// Make zoomTo() ignore the back-signal.
	emit zoomLevelChanged(level);
	centerOn(sceneRect().center());	// Center the view of the whole puzzle.
	emit zoomAdjustable(true);	// Enable the ZoomWidget.

	// Explain autosaving.
	KMessageBox::information(window(), i18n("Your progress is saved automatically while you play."), i18nc("used as caption for a dialog that explains the autosave feature", "Automatic saving"), QLatin1String("autosave-introduction"));
	qDebug() << "EXITING View::puzzleStarted()";
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

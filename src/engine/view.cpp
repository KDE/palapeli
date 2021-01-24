/*
    SPDX-FileCopyrightText: 2009, 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "view.h"
#include "interactormanager.h"
#include "scene.h"
#include "piece.h"
#include "texturehelper.h"

#include <cmath>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QScrollBar>
#include <KLocalizedString>
#include <KMessageBox>

#include <QApplication>
#include <QDesktopWidget>

#include <QTimer>
#include "palapeli_debug.h" // IDW test.

const int Palapeli::View::MinimumZoomLevel = 0;
const int Palapeli::View::MaximumZoomLevel = 200;
const int DefaultDelta = 120;

Palapeli::View::View()
	: m_interactorManager(new Palapeli::InteractorManager(this))
	, m_scene(nullptr)
	, m_zoomLevel(MinimumZoomLevel)
	, m_closeUpLevel(MaximumZoomLevel)
	, m_distantLevel(MinimumZoomLevel)
	, m_isCloseUp(false)
	, m_dZoom(20.0)
	, m_minScale(0.01)
	, m_adjustPointer(false)
{
	setFrameStyle(QFrame::NoFrame);
	setMouseTracking(true);
	setResizeAnchor(QGraphicsView::AnchorUnderMouse);
	setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	setScene(new Palapeli::Scene(this));
	connect(m_scene, &Palapeli::Scene::sceneRectChanged, this, &View::logSceneChange);
	qCDebug(PALAPELI_LOG) << "Initial size of Palapeli::View" << size();
}

// IDW test.
void Palapeli::View::logSceneChange(const QRectF &r)
{
    Q_UNUSED(r);
	// qCDebug(PALAPELI_LOG) << "View::logSceneChange" << r << "View size" << this->size();
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

void Palapeli::View::teleportPieces(Piece* pieceUnder, const QPointF& scenePos)
{
	qCDebug(PALAPELI_LOG) << "TELEPORT: pieceUnder" << (pieceUnder != nullptr)
		 << "scenePos" << scenePos;
	Q_EMIT teleport(pieceUnder, scenePos, this);
}

void Palapeli::View::zoomBy(int delta)
{
	// Scroll wheel and touchpad come here.
	// Delta is typically +-120 per click for a mouse-wheel, but can be <10
	// for an Apple MacBook touchpad (using two fingers to scroll).

	// IDW TODO - Accept deltas of <10, either by accumulating deltas or by
	//            implementing fractional zoom levels.
	qCDebug(PALAPELI_LOG) << "View::zoomBy: delta" << delta;
	m_adjustPointer = true;
	zoomTo(m_zoomLevel + delta / 10);
}

void Palapeli::View::zoomTo(int level)
{
	// IDW TODO - BUG: If you zoom out as far as Palapeli will go, using the
	//            scroll-wheel, then go on scrolling, the view will zoom in
	//            and back out again momentarily.

	// Validate/normalize input.
	level = qBound(MinimumZoomLevel, level, MaximumZoomLevel);
	// Skip unimportant requests.
	if (level == m_zoomLevel) {
		return;
	}
	// Save the mouse position in both view and scene.
	m_mousePos = mapFromGlobal(QCursor::pos());
	m_scenePos = mapToScene(m_mousePos);
	// Create a new transform.
	const qreal scalingFactor = m_minScale * pow(2, level/m_dZoom);
	qCDebug(PALAPELI_LOG) << "View::zoomTo: level" << level
		 << "scalingFactor" << scalingFactor
		 << m_mousePos << m_scenePos;
	// Translation, shear, etc. are the same: only the scale is replaced.
	QTransform t = transform();
	t.setMatrix(scalingFactor, t.m12(), t.m13(),
		    t.m21(), scalingFactor, t.m23(),
		    t.m31(), t.m32(), t.m33());
	setTransform(t);
	// Save and report changes.
	m_zoomLevel = level;
	Q_EMIT zoomLevelChanged(m_zoomLevel);
	// In a mouse-centered zoom, lock the pointer onto the scene position.
	if (m_adjustPointer) {
		// Let the new view settle down before checking the mouse.
		QTimer::singleShot(0, this, &View::adjustPointer);
	}
}

void Palapeli::View::adjustPointer()
{
	// If the view moved, keep the mouse at the same position in the scene.
	const QPoint mousePos = mapFromScene(m_scenePos);
	if (mousePos != m_mousePos) {
		qCDebug(PALAPELI_LOG) << "POINTER MOVED from" << m_mousePos
			 << "to" << mousePos << "scenePos" << m_scenePos;
		QCursor::setPos(mapToGlobal(mousePos));
	}
}

void Palapeli::View::zoomSliderInput(int level)
{
	if (level == m_zoomLevel) {
		return;		// Avoid echo from zoomLevelChanged() signal.
	}
	m_adjustPointer = false;
	zoomTo(level);
}

void Palapeli::View::zoomIn()
{
	// ZoomWidget ZoomIn button comes here via zoomInRequest signal.
	// ZoomIn menu and shortcut come here via GamePlay::actionZoomIn.
	m_adjustPointer = false;
	zoomTo(m_zoomLevel + DefaultDelta / 10);
}

void Palapeli::View::zoomOut()
{
	// ZoomWidget ZoomOut button comes here via zoomOutRequest signal.
	// ZoomOut menu and shortcut come here via GamePlay::actionZoomOut.
	m_adjustPointer = false;
	zoomTo(m_zoomLevel - DefaultDelta / 10);
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
	m_isCloseUp = !m_isCloseUp;	// Switch to the other view.
	m_adjustPointer = true;
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

void Palapeli::View::setCloseUp(bool onOff)
{
	m_isCloseUp = onOff;
	// Force zoomTo() to recalculate, even if m_zoomLevel == required value.
	m_zoomLevel = m_isCloseUp ? m_closeUpLevel - 1 : m_distantLevel + 1;
	if (m_isCloseUp) {
		zoomTo(m_closeUpLevel);
	}
	else {
		zoomTo(m_distantLevel);
	}
}

void Palapeli::View::handleNewPieceSelection()
{
	Q_EMIT newPieceSelectionSeen(this);
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

int Palapeli::View::calculateZoomRange(qreal distantScale, bool distantView)
{
	qreal closeUpScale = calculateCloseUpScale();
	if (closeUpScale < distantScale) {
		closeUpScale = distantScale;	// View is already large enough.
	}
	qCDebug(PALAPELI_LOG) << "View::calculateZoomRange: distantScale" << distantScale
		 << "distantView" << distantView
		 << "closeUpScale" << closeUpScale;
	const qreal minScale = distantScale*0.75;
	const qreal maxScale = closeUpScale*2.0;
	const qreal range = log(maxScale/minScale)/log(2.0);
	const qreal dZoom = (MaximumZoomLevel - MinimumZoomLevel)/range;
	qCDebug(PALAPELI_LOG) << "minScale" << minScale << "maxScale" << maxScale
		 << "range" << range << "dZoom" << dZoom;
	m_dZoom = dZoom;
	m_minScale = minScale;

	// Set the toggling levels. If close-up is too small, adjust it.
	m_distantLevel = qRound(dZoom*log(distantScale/minScale)/log(2.0));;
	m_closeUpLevel = qRound(MaximumZoomLevel - MinimumZoomLevel - m_dZoom);
	m_closeUpLevel = (m_closeUpLevel >= (m_distantLevel + MinDiff)) ?
				m_closeUpLevel : m_distantLevel + MinDiff;
	m_isCloseUp = (! distantView);	// Start with the view zoomed in or out.
	const int level = (distantView ? m_distantLevel : m_closeUpLevel);
	qCDebug(PALAPELI_LOG) << "INITIAL LEVEL" << level
		 << "toggles" << m_distantLevel << m_closeUpLevel;
	return level;
}

void Palapeli::View::puzzleStarted()
{
	qCDebug(PALAPELI_LOG) << "ENTERED View::puzzleStarted()";
	// At this point the puzzle pieces have been shuffled or loaded from a
	// .save file and the puzzle table has been scaled to fit the view. Now
	// adjust zooming and slider to a range of distant and close-up views.

	// Choose the lesser of the horizontal and vertical scaling factors.
	const qreal distantScale = qMin(transform().m11(), transform().m22());
	qCDebug(PALAPELI_LOG) << "distantScale" << distantScale;
	// Calculate the zooming range and return the distant scale's level.
	int level = calculateZoomRange(distantScale, true);

	// Don't readjust the zoom. Just set the slider pointer.
	m_zoomLevel = level;		// Make zoomTo() ignore the back-signal.
	Q_EMIT zoomLevelChanged(level);
	centerOn(sceneRect().center());	// Center the view of the whole puzzle.
	Q_EMIT zoomAdjustable(true);	// Enable the ZoomWidget.

	// Explain autosaving.
	KMessageBox::information(window(), i18n("Your progress is saved automatically while you play."), i18nc("used as caption for a dialog that explains the autosave feature", "Automatic saving"), QStringLiteral("autosave-introduction"));
	qCDebug(PALAPELI_LOG) << "EXITING View::puzzleStarted()";
}

void Palapeli::View::startVictoryAnimation()
{
	//move viewport to show the complete puzzle
	QPropertyAnimation* animation = new QPropertyAnimation(this, "viewportRect", this);
	animation->setEndValue(m_scene->extPiecesBoundingRect());
	animation->setDuration(1000);
	animation->start(QAbstractAnimation::DeleteWhenStopped);
	Q_EMIT zoomAdjustable(false);
}



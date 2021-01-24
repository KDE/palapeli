/*
    SPDX-FileCopyrightText: 2010 Johannes Loehnert <loehnert.kde@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "puzzlepreview.h"

#include "../file-io/puzzlestructs.h"
#include "settings.h"

#include <cmath>
#include <QMouseEvent>
#include <QPainter>
#include <KLocalizedString>

Palapeli::PuzzlePreview::PuzzlePreview(QWidget* parent)
{
	m_settingsSaveTimer = new QTimer(this);
	connect(m_settingsSaveTimer, &QTimer::timeout, this, &PuzzlePreview::writeConfigIfGeometryChanged);
	m_geometryChanged = false;

	m_hoverZoom = 1.0;
	m_isZoomed = false;
	m_mousePos = QPoint();

	setScene(new QGraphicsScene(this));
	setParent(parent);
	setWindowTitle(i18nc("Window title", "Preview of completed puzzle"));
	setWindowFlags(Qt::Tool | Qt::WindowTitleHint);
	setAttribute (Qt::WA_NoMousePropagation); // Accept all mouse events.
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setRenderHint(QPainter::SmoothPixmapTransform);
	scene()->addText(i18nc("text in preview window",
		"Image is not available."));	// Seen if image-data not found.
	setSceneRect(scene()->itemsBoundingRect());

	// read size and position settings
	QRect geometry = Settings::puzzlePreviewGeometry();
	resize(geometry.size());

	// default (-1/-1) toprect: don't change position
	if (geometry.left() >= 0 && geometry.top() >= 0)
		move(geometry.topLeft());

	m_settingsSaveTimer->start(500);
	hide();
	updateViewport();
}

void Palapeli::PuzzlePreview::setImage(const QImage &image)
{
	scene()->clear();
	scene()->addPixmap(QPixmap::fromImage(image));
	setSceneRect(image.rect());
	updateViewport();
}

void Palapeli::PuzzlePreview::loadImageFrom(const Palapeli::PuzzleMetadata& md)
{
	// Metadata is assumed to have been loaded by the caller.
	setImage(md.image);
	setWindowTitle(i18n("%1 - Preview", md.name));
	// Set hover-zoom so that 3x3 pieces would be visible on a square grid.
	m_hoverZoom = sqrt(md.pieceCount)/3.0;
	if (m_hoverZoom < 1)
		m_hoverZoom = 1;
}

void Palapeli::PuzzlePreview::toggleVisible()
{
	setVisible(!isVisible());
	Settings::setPuzzlePreviewVisible(isVisible());
	Settings::self()->save();
}

void Palapeli::PuzzlePreview::mouseMoveEvent(QMouseEvent* event)
{
	m_mousePos = event->pos();
	updateViewport();
	QGraphicsView::mouseMoveEvent(event);
}

void Palapeli::PuzzlePreview::enterEvent(QEvent* event)
{
	setMouseTracking(true);
	m_isZoomed = true;
	m_mousePos = QPoint();
	// wait with update for first mouseMoveEvent
	QGraphicsView::enterEvent(event);
}

void Palapeli::PuzzlePreview::leaveEvent(QEvent* event)
{
	setMouseTracking(false);
	m_isZoomed = false;
	updateViewport();
	QGraphicsView::leaveEvent(event);
}

void Palapeli::PuzzlePreview::resizeEvent(QResizeEvent* event)
{
	updateViewport();
	m_geometryChanged = true;
	QGraphicsView::resizeEvent(event);
}

void Palapeli::PuzzlePreview::moveEvent(QMoveEvent* event)
{
	m_geometryChanged = true;
	QGraphicsView::moveEvent(event);
}

void Palapeli::PuzzlePreview::closeEvent(QCloseEvent* event)
{
	// Triggered by the preview window's Close button.
	event->accept();
	Q_EMIT closing();
}

void Palapeli::PuzzlePreview::writeConfigIfGeometryChanged()
{
	if (!m_geometryChanged) return;

	// move() includes window frame, resize() doesn't :-/
	Settings::setPuzzlePreviewGeometry(QRect(frameGeometry().topLeft(), size()));
	Settings::self()->save();
	m_geometryChanged = false;
}

void Palapeli::PuzzlePreview::updateViewport()
{
	qreal zoom;
	// calculate zoom for fit-in-window
	zoom = width() / sceneRect().width();
	if (zoom > height() / sceneRect().height())
		zoom = height() / sceneRect().height();

	if (m_isZoomed)
		zoom *= m_hoverZoom;

	// do not enlarge
	if (zoom>1)
		zoom = 1;

	resetTransform();
	scale(zoom, zoom);

	if (m_isZoomed)
	{
		// focus moves with cursor position
		QPointF pos = m_mousePos;
		pos.rx() *= sceneRect().width() / width();
		pos.ry() *= sceneRect().height() / height();
		centerOn(pos);
	}
}



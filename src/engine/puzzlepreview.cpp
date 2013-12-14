/***************************************************************************
 *   Copyright 2010 Johannes Loehnert <loehnert.kde@gmx.de>
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

#include "puzzlepreview.h"
#include "../file-io/collection.h"
#include "../file-io/puzzle.h"
#include "settings.h"

#include <cmath>
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>
#include <KLocalizedString>

Palapeli::PuzzlePreview::PuzzlePreview()
{
	m_settingsSaveTimer = new QTimer(this);
	connect(m_settingsSaveTimer, SIGNAL(timeout()), this, SLOT(writeConfigIfGeometryChanged()));
	m_geometryChanged = false;
	
	m_hoverZoom = 1.0;
	m_isZoomed = false;
	m_mousePos = QPoint();
	
	setScene(new QGraphicsScene());
	setWindowTitle(i18nc("tool window title", "Preview of assembled puzzle"));
	setWindowFlags(Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setRenderHint(QPainter::SmoothPixmapTransform);
	scene()->addText(i18nc("text in preview window", "No puzzle is loaded."));
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

void Palapeli::PuzzlePreview::loadImageFrom(const QModelIndex &index)
{
	QObject* puzzlePayload = index.data(Palapeli::Collection::PuzzleObjectRole).value<QObject*>();
	Palapeli::Puzzle* puzzle = qobject_cast<Palapeli::Puzzle*>(puzzlePayload);
	if (puzzle && m_puzzle != puzzle)
	{
		m_puzzle = puzzle;
		loadImageFromInternal(puzzle);
	}
}

void Palapeli::PuzzlePreview::loadImageFromInternal(Palapeli::Puzzle *puzzle)
{
	// metadata was assumedly loaded by Palapeli::Scene.
	// FIXME: possible race condition? in case of doubt, how to wait for metadata?
	// (would have to catch Scene::m_metadataLoader finished() --> bad design!)
	// (or, use a timer and look every x ms if we can proceed)
	if (puzzle->metadata())
	{
		setImage(puzzle->metadata()->image);
		
		// set hover zoom so that 3x3 pieces would be visible on a square grid.
		m_hoverZoom = sqrt(puzzle->metadata()->pieceCount)/3.0;
		if (m_hoverZoom < 1)
			m_hoverZoom = 1;
	}
	else
	{
		qWarning() << "cannot set image: metadata not loaded";
	}
}

void Palapeli::PuzzlePreview::toggleVisible()
{
	setVisible(!isVisible());
	Settings::setPuzzlePreviewVisible(isVisible());
	Settings::self()->writeConfig();
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

void Palapeli::PuzzlePreview::writeConfigIfGeometryChanged()
{
	if (!m_geometryChanged) return;
	
	qDebug() << "puzzle preview: saving changed geometry";
	
	// move() includes window frame, resize() doesn't :-/
	Settings::setPuzzlePreviewGeometry(QRect(frameGeometry().topLeft(), size()));
	Settings::self()->writeConfig();
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

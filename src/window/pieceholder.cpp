/***************************************************************************
 *   Copyright 2014 Ian Wadham <iandw.au@gmail.com>
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

#include "pieceholder.h"
#include "../engine/scene.h"
#include "../engine/piece.h"
#include "settings.h"

#include <QCloseEvent>
#include "palapeli_debug.h"

const qreal minGrid = 2.0;	// 2x2 pieces in close-up of minimum holder.
const qreal maxGrid = 6.0;	// 6x6 pieces in distant view of min holder.

Palapeli::PieceHolder::PieceHolder(QWidget* parent, const QSizeF& pieceArea,
					const QString& title)
	: m_scene(scene())
{
	qCDebug(PALAPELI_LOG) << "CONSTRUCTING Palapeli::PieceHolder" << title;
	setParent(parent);
	setWindowFlags(Qt::Window | Qt::Tool | Qt::WindowTitleHint);
	// Allow space for (2 * 2) pieces in minimum view initially.
	m_scene->setPieceAreaSize(pieceArea);
	m_scene->initializeGrid(QPointF(0.0, 0.0));
	m_scene->setMinGrid(minGrid);
	// Add margin for constraint_handles+spacer and setSceneRect().
	QRectF rect = m_scene->piecesBoundingRect();
	qreal handleWidth = qMin(rect.width(), rect.height())/25.0;
	m_scene->addMargin(handleWidth, 0.5*handleWidth);
	setWindowTitle(title);
	qreal s = calculateCloseUpScale();
	QRectF r = m_scene->sceneRect();
	setMinimumSize(s*r.width()+1.0, s*r.height()+1.0);
	resize(minimumSize());
	qCDebug(PALAPELI_LOG) << "Close-up scale" << s << "pieceArea" << pieceArea
		 << "size" << size();
	QTransform t;
	t.scale(s, s);
	setTransform(t);
	centerOn(r.center());
	setSelected(true);
	show();
}

void Palapeli::PieceHolder::initializeZooming()
{
	// Allow space for a distant view of at most (maxGrid * maxGrid) pieces
	// in a piece holder when the view is at minimum size. More that number
	// of pieces can be teleported in, but the holder window will have to be
	// resized or scrolled for the user to see them, even in distant view.

	qCDebug(PALAPELI_LOG) << "ENTERED PieceHolder::initializeZooming() for" << name();
	qreal scale = qMin(transform().m11(), transform().m22());
	scale = scale * (minGrid / maxGrid);
	// Calculate the zooming range and return the close-up scale's level.
	int level = calculateZoomRange(scale, false);
	zoomTo(level);
	centerOn(sceneRect().center());
}

void Palapeli::PieceHolder::focusInEvent(QFocusEvent* e)
{
	// The user selected this piece holder.
	Q_UNUSED(e)
	qCDebug(PALAPELI_LOG) << "PieceHolder::focusInEvent()" << windowTitle();
	setSelected(true);
	emit selected(this);	// De-select the previously selected holder.
}

void Palapeli::PieceHolder::setSelected(bool onOff)
{
	qCDebug(PALAPELI_LOG) << "PieceHolder::setSelected()" << windowTitle() << onOff;
	setStyleSheet(QString("QFrame { border: 3px solid %1; }").arg
				(onOff ? "blue" : "lightGray"));
}

void Palapeli::PieceHolder::closeEvent(QCloseEvent* event)
{
	// Triggered by the piece-holder window's Close button.
	if(m_scene->pieces().isEmpty()) {
		event->accept();	// The window can be closed.
	}
	else {
		event->ignore();	// The window cannot be closed.
	}
	emit closing(this);		// GamePlay handles the details.
}



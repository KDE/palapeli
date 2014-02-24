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
#include <QDebug>

Palapeli::PieceHolder::PieceHolder(const QSizeF& pieceArea, const QString& title)
	: View()		// A parentless QWidget == a window.
	, m_scene(scene())
{
	// Allow space for four pieces initially.
	qreal f = (1.0 + 0.05 * Settings::pieceSpacing()) * 2.0;
	QRectF r(QPointF(0.0, 0.0), pieceArea * f);
	m_scene->setPieceAreaSize(pieceArea);
	m_scene->initializeGrid(QPointF(0.0, 0.0));
	m_scene->setSceneRect(r);
	setWindowFlags(Qt::Tool | Qt::WindowTitleHint
				| Qt::WindowStaysOnTopHint);
	setWindowTitle(title);
	qreal s = calculateCloseUpScale();
	setMinimumSize(s*f*pieceArea.width()+1.0, s*f*pieceArea.height()+1.0);
	resize(minimumSize());
	QTransform t;
	t.scale(s, s);
	setTransform(t);
	centerOn(r.center());
	setSelected(true);
	show();
}

void Palapeli::PieceHolder::initializeZooming()
{
	qDebug() << "ENTERED PieceHolder::initializeZooming() for" << name();
}

void Palapeli::PieceHolder::focusInEvent(QFocusEvent* e)
{
	// The user selected this piece holder.
	Q_UNUSED(e)
	qDebug() << "PieceHolder::focusInEvent()" << windowTitle();
	setSelected(true);
	emit selected(this);	// De-select the previously selected holder.
}

void Palapeli::PieceHolder::setSelected(bool onOff)
{
	qDebug() << "PieceHolder::setSelected()" << windowTitle() << onOff;
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

#include "pieceholder.moc"

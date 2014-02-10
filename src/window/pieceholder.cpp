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

#include <QDebug>

Palapeli::PieceHolder::PieceHolder(const QSizeF& pieceArea, const QString& title)
	: View()		// A parentless QWidget == a window.
	, m_scene(scene())
	, m_rank(1)
	, m_x(0)
	, m_y(0)
{
	QRectF r(QPointF(0.0, 0.0), pieceArea);
	m_scene->setPieceAreaSize(pieceArea);
	m_scene->setSceneRect(r);
	setWindowFlags(Qt::Tool | Qt::WindowTitleHint
				| Qt::WindowStaysOnTopHint);
	setWindowTitle(title);
	qreal s = calculateCloseUpScale();
	qreal f = 1.5;
	setMinimumSize(s*f*pieceArea.width()+1.0, s*f*pieceArea.height()+1.0);
	resize(minimumSize());
	QTransform t;
	t.scale(s, s);
	setTransform(t);
	centerOn(r.center());
	setSelected(true);
	show();
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

void Palapeli::PieceHolder::receivePieces(const QList<Palapeli::Piece*> pieces)
{
	foreach (Palapeli::Piece* piece, pieces) {
		m_scene->addPieceToList(piece);
		m_scene->addItem(piece);
		piece->setPlace(m_x, m_y, m_scene->pieceAreaSize(), false);
		// Allow pieces to be merged and joined within the piece holder.
		connect(piece, SIGNAL(moved(bool)),
			m_scene, SLOT(pieceMoved(bool)));

		// Calculate the next spot on the square grid.
		if (m_y == (m_rank - 1)) {
			m_x++;			// Add to bottom row.
			if (m_x > (m_rank - 1)) {
				m_rank++;	// Expand the square grid.
				m_x = m_rank - 1;
				m_y = 0;	// Start right-hand column.
			}
		}
		else {
			m_y++;			// Add to right-hand column.
			if (m_y == (m_rank - 1)) {
				m_x = 0;	// Start bottom row.
			}
		}
	}
	m_scene->setSceneRect(m_scene->piecesBoundingRect());
	centerOn(pieces.last()->sceneBareBoundingRect().center());
}

void Palapeli::PieceHolder::unloadAllPieces(Palapeli::Scene* dest)
{
	const QRectF r = m_scene->piecesBoundingRect();
	const QRectF d = ((QGraphicsScene*)dest)->sceneRect();
	const QPointF c = d.center();
	const QPointF delta = d.topLeft() - r.topLeft() +
				c - QPointF(r.width()/2.0, r.height()/2.0);
	QList<Palapeli::Piece*> pieces = m_scene->pieces();
	m_scene->dispatchPieces(m_scene->pieces());
	foreach (Palapeli::Piece* piece, pieces) {
		dest->addPieceToList(piece);
		dest->addItem(piece);
		piece->setPos(piece->pos() + delta);
		piece->setSelected(true);
		connect(piece, SIGNAL(moved(bool)),
			dest, SLOT(pieceMoved(bool)));
	}

	// Re-initialize the grid.
	m_rank = 1;
	m_x = 0;
	m_y = 0;
}

#include "pieceholder.moc"

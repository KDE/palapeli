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

#include <QDebug>

Palapeli::PieceHolder::PieceHolder(const QSizeF& pieceArea, const QString& title)
	: View()		// A parentless QWidget == a window.
{
	scene()->setPieceAreaSize(pieceArea);
	setWindowFlags(Qt::Tool | Qt::WindowTitleHint
				| Qt::WindowStaysOnTopHint);
	setWindowTitle(title);
	setMinimumSize(pieceArea.width()+1.0, pieceArea.height()+1.0);
	resize(minimumSize());
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

#include "pieceholder.moc"

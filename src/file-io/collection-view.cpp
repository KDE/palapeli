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

#include "collection-view.h"
#include "collection.h"
#include "collection-delegate.h"

#include <QApplication>

Palapeli::CollectionView::CollectionView()
	: m_delegate(new Palapeli::CollectionDelegate(this))
{
	connect(this, SIGNAL(activated(const QModelIndex&)), this, SLOT(handleActivated(const QModelIndex&)));
	setAlternatingRowColors(true);
	setMouseTracking(true);
}

//NOTE The QAbstractItemView::activated signal honors the mouseclick selection behavior defined by the user (e.g. one-click is default on Linux, while two-click is default on Windows).
void Palapeli::CollectionView::handleActivated(const QModelIndex& index)
{
	//do not emit a play request when the Control modifier is pressed (without this rule, users with one-click activation could not ever select multiple puzzles at once)
	if (QApplication::keyboardModifiers() & Qt::ControlModifier)
		return;
	//change selection to indicate that the given puzzle has been chosen
	selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
	emit playRequest(index);
}

#include "collection-view.moc"

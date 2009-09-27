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

#include "libraryview.h"
#include "librarydelegate.h"
#include "librarymodel.h"

Palapeli::LibraryView::LibraryView(QWidget* parent)
	: m_model(new Palapeli::LibraryModel(this))
{
	setModel(m_model);
	new Palapeli::LibraryDelegate(this);
	connect(this, SIGNAL(activated(const QModelIndex&)), this, SLOT(handleActivated(const QModelIndex&)));
}

void Palapeli::LibraryView::handleActivated(const QModelIndex& index)
{
	emit selected(m_model->puzzle(index));
}

#include "libraryview.moc"

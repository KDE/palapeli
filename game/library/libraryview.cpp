/***************************************************************************
 *   Copyright (C) 2008 Stefan Majewsky <majewsky@gmx.net>
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
#include "library.h"
#include "librarydelegate.h"
#include "libraryfilter.h"

Palapeli::LibraryView::LibraryView(Palapeli::Library* library)
	: m_library(library)
	, m_delegate(new Palapeli::LibraryDelegate)
	, m_filter(0)
{
	setModel(m_library);
	setItemDelegate(m_delegate);
	setSelectionMode(QAbstractItemView::SingleSelection);
}

Palapeli::LibraryView::~LibraryView()
{
	delete m_delegate;
	delete m_filter;
}

Palapeli::Library* Palapeli::LibraryView::library() const
{
	return m_library;
}

Palapeli::PuzzleInfo* Palapeli::LibraryView::puzzleInfo() const
{
	const QModelIndexList indexes = selectionModel()->selectedIndexes();
	if (indexes.isEmpty())
		return 0;
	else
	{
		const QModelIndex index = m_filter ? m_filter->mapToSource(indexes[0]) : indexes[0];
		return m_library->infoForPuzzle(index);
	}
}

void Palapeli::LibraryView::setDeletionFilterEnabled(bool enable)
{
	if (enable && !m_filter)
	{
		m_filter = new Palapeli::LibraryFilter(m_library);
		setModel(m_filter);
	}
	if (!enable && m_filter)
	{
		setModel(m_library);
		delete m_filter;
		m_filter = 0;
	}
}

#include "libraryview.moc"
/***************************************************************************
 *   Copyright 2008 Stefan Majewsky <majewsky@gmx.net>
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

#include "libraryfilter.h"
#include "library.h"
#include "librarybase.h"
#include "puzzleinfo.h"

Palapeli::LibraryFilter::LibraryFilter(Palapeli::Library* source)
	: QSortFilterProxyModel()
	, m_source(source)
{
	setSourceModel(source);
	setDynamicSortFilter(true);
}

bool Palapeli::LibraryFilter::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
	const QModelIndex index = m_source->index(sourceRow, 0, sourceParent);
	Palapeli::PuzzleInfo* info = m_source->infoForPuzzle(index);
	return m_source->base()->canRemoveEntry(info->identifier, m_source);
}

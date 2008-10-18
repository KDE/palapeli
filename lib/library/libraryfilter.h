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

#ifndef PALAPELI_LIBRARYFILTER_H
#define PALAPELI_LIBRARYFILTER_H

#include "../macros.h"
#include <QSortFilterProxyModel>

namespace Palapeli
{

	class Library;

	class PALAPELIBASE_EXPORT LibraryFilter : public QSortFilterProxyModel
	{
		public:
			LibraryFilter(Palapeli::Library* source);
			virtual bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const;
		private:
			Palapeli::Library* m_source;
	};

}

#endif // PALAPELI_LIBRARYFILTER_H

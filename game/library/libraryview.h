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

#ifndef PALAPELI_LIBRARYVIEW_H
#define PALAPELI_LIBRARYVIEW_H

#include <QListView>

namespace Palapeli
{

	class Library;
	class LibraryDelegate;
	class PuzzleInfo;

	class LibraryView : public QListView
	{
		Q_OBJECT
		public:
			explicit LibraryView(Palapeli::Library* library);
			~LibraryView();

			Palapeli::Library* library() const;
			Palapeli::PuzzleInfo* puzzleInfo() const;
		private:
			Palapeli::Library* m_library;
			Palapeli::LibraryDelegate* m_delegate;
	};

}

#endif // PALAPELI_LIBRARYVIEW_H

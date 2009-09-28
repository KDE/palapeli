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

#ifndef PALAPELI_MAINWINDOW_H
#define PALAPELI_MAINWINDOW_H

class KTabWidget;
#include <KXmlGuiWindow>

namespace Palapeli
{
	class LibraryView;
	class PuzzleReader;
	class PuzzleTableWidget;

	class MainWindow : public KXmlGuiWindow
	{
		Q_OBJECT
		public:
			MainWindow();
		public Q_SLOTS:
			void loadPuzzle(Palapeli::PuzzleReader* puzzle);
		private:
			KTabWidget* m_centralWidget;
			Palapeli::PuzzleTableWidget* m_puzzleTable;
			Palapeli::LibraryView* m_library;
	};
}

#endif // PALAPELI_MAINWINDOW_H

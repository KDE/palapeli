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

#ifndef PALAPELI_PUZZLETABLEWIDGET_H
#define PALAPELI_PUZZLETABLEWIDGET_H

#include "tabwindow.h"

namespace Palapeli
{
	class PuzzleReader;
	class TextProgressBar;
	class View;

	class PuzzleTableWidget : public Palapeli::TabWindow
	{
		Q_OBJECT
		public:
			PuzzleTableWidget();
		public Q_SLOTS:
			void reportProgress(int pieceCount, int partCount);
			void loadPuzzle(Palapeli::PuzzleReader* puzzle);
		private:
			Palapeli::View* m_view;
			Palapeli::TextProgressBar* m_progressBar;
	};
}

#endif // PALAPELI_PUZZLETABLE_H

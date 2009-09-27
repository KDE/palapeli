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

#ifndef PALAPELI_LIBRARYVIEW_H
#define PALAPELI_LIBRARYVIEW_H

#include <QListView>

namespace Palapeli
{
	class LibraryModel;
	class PuzzleReader;

	class LibraryView : public QListView
	{
		Q_OBJECT
		public:
			LibraryView(QWidget* parent = 0);
		Q_SIGNALS:
			void selected(Palapeli::PuzzleReader* puzzle);
		private Q_SLOTS:
			void handleActivated(const QModelIndex& index);
		private:
			Palapeli::LibraryModel* m_model;
	};
}

#endif // PALAPELI_LIBRARYVIEW_H

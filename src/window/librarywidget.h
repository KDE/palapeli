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

#ifndef PALAPELI_LIBRARYWIDGET_H
#define PALAPELI_LIBRARYWIDGET_H

#include "tabwindow.h"

class QListView;
class QModelIndex;
class KAction;

namespace Palapeli
{
	class LibraryModel;
	class Puzzle;

	class LibraryWidget : public Palapeli::TabWindow
	{
		Q_OBJECT
		public:
			LibraryWidget();

			Palapeli::LibraryModel* model() const;
		Q_SIGNALS:
			void playRequest(Palapeli::Puzzle* puzzle);
		protected:
			virtual void resizeEvent(QResizeEvent* event);
		private Q_SLOTS:
			void handleDeleteRequest();
			void handleExportRequest();
			void handleImportRequest();
			void handlePlayRequest(const QString& puzzleIdentifier);
			void handleSelectionChanged();
		private:
			QListView* m_view;
			Palapeli::LibraryModel* m_model;
			KAction* m_exportAct;
			KAction* m_deleteAct;
	};
}

#endif // PALAPELI_LIBRARYWIDGET_H

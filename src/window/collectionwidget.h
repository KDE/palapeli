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

#ifndef PALAPELI_COLLECTIONWIDGET_H
#define PALAPELI_COLLECTIONWIDGET_H

#include "tabwindow.h"

class QModelIndex;
class KAction;
class KUrl;

namespace Palapeli
{
	class ListCollection;
	class CollectionView;
	class FileSystemCollection;
	class OldPuzzle;

	class CollectionWidget : public Palapeli::TabWindow
	{
		Q_OBJECT
		public:
			CollectionWidget();

			void startPuzzle(const KUrl& url);
			QModelIndex storeGeneratedPuzzle(Palapeli::OldPuzzle* puzzle);
		Q_SIGNALS:
			void createRequest();
			void playRequest(const QModelIndex& index);
		private Q_SLOTS:
			void handleDeleteRequest();
			void handleExportRequest();
			void handleImportRequest();
			void handleSelectionChanged();
		private:
			Palapeli::CollectionView* m_view;
			Palapeli::ListCollection* m_localCollection;
			Palapeli::FileSystemCollection* m_fsCollection;
			KAction* m_exportAct;
			KAction* m_deleteAct;
	};
}

#endif // PALAPELI_COLLECTIONWIDGET_H

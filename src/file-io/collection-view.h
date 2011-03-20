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

#ifndef PALAPELI_COLLECTIONVIEW_H
#define PALAPELI_COLLECTIONVIEW_H

#include <QListView>
class QSortFilterProxyModel;
class QAction;

namespace Palapeli
{
	class CollectionDelegate;

	class CollectionView : public QWidget
	{
		Q_OBJECT
		public:
			CollectionView(QWidget* parent = 0);

			void setModel(QAbstractItemModel* model);
			QItemSelectionModel* selectionModel() const;
		Q_SIGNALS:
			void playRequest(const QModelIndex&);
		private Q_SLOTS:
			void handleActivated(const QModelIndex& index);
			void sortMenuTriggered(QAction* action);
		private:
			QListView* m_view;
			Palapeli::CollectionDelegate* m_delegate;
			QSortFilterProxyModel* m_proxyModel;
			QAction* m_sortByTitle;
			QAction* m_sortByPieceCount;
	};
}

#endif // PALAPELI_COLLECTIONVIEW_H

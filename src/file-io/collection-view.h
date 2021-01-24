/*
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELI_COLLECTIONVIEW_H
#define PALAPELI_COLLECTIONVIEW_H

#include <QListView>
class QSortFilterProxyModel;
class QAction;

namespace Palapeli
{
	class CollectionDelegate;
	class Puzzle;

	class CollectionView : public QWidget
	{
		Q_OBJECT
		public:
			explicit CollectionView(QWidget* parent = nullptr);

			void setModel(QAbstractItemModel* model);
			QModelIndexList selectedIndexes() const;
		Q_SIGNALS:
			void canDeleteChanged(bool canDelete);
			void canExportChanged(bool canExport);
			void playRequest(Palapeli::Puzzle* puzzle);
		private Q_SLOTS:
			void handleActivated(const QModelIndex& index);
			void handleSelectionChanged();
			void sortMenuTriggered(QAction* action);
			void slotTextChanged(const QString &str);
		private:
			QListView* m_view;
			Palapeli::CollectionDelegate* m_delegate;
			QSortFilterProxyModel* m_proxyModel;
			QAction* m_sortByTitle;
			QAction* m_sortByPieceCount;
	};
}

#endif // PALAPELI_COLLECTIONVIEW_H

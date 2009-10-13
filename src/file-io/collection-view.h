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
class QToolButton;

namespace Palapeli
{
	class CollectionDelegate;

	class CollectionView : public QListView
	{
		Q_OBJECT
		public:
			CollectionView();
		Q_SIGNALS:
			void playRequest(const QModelIndex&);
		protected:
			virtual bool eventFilter(QObject* object, QEvent* event);
			void setHoveredIndex(const QModelIndex& index);
		private Q_SLOTS:
			void playButtonClicked();
		private:
			Palapeli::CollectionDelegate* m_delegate;
			QString m_hoveredPuzzleIdentifier;
			QToolButton* m_playButton;
	};
}

#endif // PALAPELI_COLLECTIONVIEW_H

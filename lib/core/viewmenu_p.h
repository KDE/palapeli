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

#ifndef PALAPELI_VIEWMENU_PRIVATE_H
#define PALAPELI_VIEWMENU_PRIVATE_H

#include <QBrush>
#include <QGridLayout>
#include <QPixmap>
#include <QPushButton>
#include <QWidget>

namespace Palapeli
{

	class ViewMenuItem : public QPushButton
	{
		Q_OBJECT
		private:
			QString m_fileName;
			QBrush m_pixmapBrush;
		public:
			ViewMenuItem(const QString& fileName, const QPixmap& pixmap);

			QString fileName() const { return m_fileName; }
			QBrush pixmapBrush() const { return m_pixmapBrush; }
		Q_SIGNALS:
			void clicked(Palapeli::ViewMenuItem* item);
			void hoverEntered(Palapeli::ViewMenuItem* item);
		private Q_SLOTS:
			void handleClicked();
		protected:
			virtual void enterEvent(QEvent* event);
	};

	class ViewMenuWidget : public QWidget
	{
		Q_OBJECT
		public:
			ViewMenuWidget(const QList<Palapeli::ViewMenuItem*>& items);
		Q_SIGNALS:
			void hoverLeft();
		protected:
			virtual void leaveEvent(QEvent* event);
		private:
			QGridLayout m_layout;
	};

	class ViewMenuPrivate
	{
		public:
			ViewMenuPrivate(Palapeli::ViewMenu* parent);

			Palapeli::ViewMenuWidget* m_container;
			QList<Palapeli::ViewMenuItem*> m_items;
			Palapeli::ViewMenuItem* m_currentItem;
	};

}

#endif // PALAPELI_VIEWMENU_PRIVATE_H

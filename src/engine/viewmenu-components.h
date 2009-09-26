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

#ifndef PALAPELI_VIEWMENU_COMPONENTS_H
#define PALAPELI_VIEWMENU_COMPONENTS_H

#include <QPushButton>

namespace Palapeli
{
	class ViewMenuItem : public QPushButton
	{
		Q_OBJECT
		public:
			ViewMenuItem(const QString& fileName);

			QBrush brush() const;
		Q_SIGNALS:
			void startPreview(const QBrush& brush);
			void selected(const QString& fileName, const QBrush& brush);
		protected:
			virtual void enterEvent(QEvent* event);
		private Q_SLOTS:
			void handleClicked();
		private:
			QBrush m_brush;
			QString m_fileName;

			static const int DefaultButtonSize;
			static const int DefaultPixmapSize;
	};

	class ViewMenuWidget : public QWidget
	{
		Q_OBJECT
		public:
			ViewMenuWidget(const QList<Palapeli::ViewMenuItem*>& items);
		Q_SIGNALS:
			void stopPreview();
		protected:
			virtual void leaveEvent(QEvent* event);
	};
}

#endif // PALAPELI_VIEWMENU_COMPONENTS_H

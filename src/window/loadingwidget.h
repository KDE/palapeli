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

#ifndef PALAPELI_LOADINGWIDGET_H
#define PALAPELI_LOADINGWIDGET_H

class QTimer;
#include <QWidget>

namespace Palapeli
{
	class LoadingWidget : public QWidget
	{
		public:
			explicit LoadingWidget(QWidget* parent = nullptr);
		protected:
			void showEvent(QShowEvent* event) override;
			void hideEvent(QHideEvent* event) override;
			void paintEvent(QPaintEvent* event) override;
		private:
			QTimer* m_updateTimer;
			int m_angleDegrees;
	};
}

#endif // PALAPELI_LOADINGWIDGET_H

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

#ifndef PALAPELI_ZOOMWIDGET_H
#define PALAPELI_ZOOMWIDGET_H

class QSlider;
class QToolButton;
#include <QWidget>

namespace Palapeli
{
	class ZoomWidget : public QWidget
	{
		Q_OBJECT
		public:
			ZoomWidget(QWidget* parent = 0);
		public Q_SLOTS:
			void setConstrained(bool constrained);
			void setLevel(qreal level);
		Q_SIGNALS:
			void constrainedChanged(bool constrained);
			void levelChanged(qreal level);
			void zoomInRequest();
			void zoomOutRequest();
		private Q_SLOTS:
			void handleValueChanged(int value);
		private:
			QToolButton* m_constrainedButton;
			QToolButton* m_zoomOutButton;
			QToolButton* m_zoomInButton;
			QSlider* m_slider;
	};
}

#endif // PALAPELI_ZOOMWIDGET_H

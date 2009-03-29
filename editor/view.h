/***************************************************************************
 *   Copyright 2008 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef PALADESIGN_VIEW_H
#define PALADESIGN_VIEW_H

#include <QWidget>

namespace Paladesign
{

	class Manager;

	class View : public QWidget
	{
		Q_OBJECT
		public:
			View(Manager* manager, QWidget* parent = 0);
		protected:
			QPointF widgetToScene(const QPoint& point);

			virtual void mousePressEvent(QMouseEvent*);
			virtual void mouseMoveEvent(QMouseEvent*);
			virtual void mouseReleaseEvent(QMouseEvent*);
			virtual void leaveEvent(QEvent*);
			virtual void paintEvent(QPaintEvent*);
			virtual void resizeEvent(QResizeEvent*);
		protected Q_SLOTS:
			void select(QObject* object);
		private:
			Manager* m_manager;
			QPoint m_offset;
			qreal m_scalingFactor;
	};

}

#endif // PALADESIGN_VIEW_H

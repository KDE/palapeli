/***************************************************************************
 *   Copyright (C) 2008 Felix Lemke <lemke.felix@ages-skripte.org>
 *   Copyright (C) 2008 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef PALAPELI_MINIMAP_H
#define PALAPELI_MINIMAP_H

#include <QWidget>

namespace Palapeli
{

	class Minimap : public QWidget
	{
		public:
			Minimap(QWidget* parent = 0);

			void setQualityLevel(int level);
		protected:
			virtual void mousePressEvent(QMouseEvent* event);
			virtual void mouseMoveEvent(QMouseEvent* event);
			virtual void mouseReleaseEvent(QMouseEvent* event);
			virtual void paintEvent(QPaintEvent*);

			QRectF viewport() const;
			QPointF widgetToScene(const QPointF& point) const;
			void moveViewport(const QPointF& widgetTo, const QPointF& widgetFrom = QPointF());
		private:
			bool m_draggingViewport, m_viewportWasDragged;
			QPoint m_draggingPreviousPos;
			int m_qualityLevel;
	};

}

#endif //PALAPELI_MINIMAP_H
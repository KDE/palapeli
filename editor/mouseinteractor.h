/***************************************************************************
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

#ifndef PALADESIGN_MOUSEINTERACTOR_H
#define PALADESIGN_MOUSEINTERACTOR_H

#include <QObject>
#include <QPointF>

namespace Paladesign
{

	class MouseInteractor : public QObject
	{
		Q_OBJECT
		public:
			MouseInteractor();

			bool hovered() const;
			void setHovered(bool hovered);
			bool selected() const;
			void setSelected(bool selected);
			bool clicked() const;
			void setClicked(bool clicked);
			QPointF mousePosition() const;
			QPointF mouseStartPosition() const;
			void setMousePosition(const QPointF& point);
			virtual bool hoverAreaContains(const QPointF& point) = 0;
			virtual bool clickAreaContains(const QPointF& point);
		protected:
			virtual void mouseDown() = 0;
			virtual void mouseMove() = 0;
			virtual void mouseUp() = 0;
			void announceInteractorChanges();
		Q_SIGNALS:
			void mouseStateChanged();
			void interactorChanged();
		private:
			QPointF m_startPosition, m_currentPosition;
			bool m_hovered, m_selected, m_clicked;
	};

}

#endif // PALADESIGN_MOUSEINTERACTOR_H

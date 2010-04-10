/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
***************************************************************************/

#ifndef PALAPELI_MOUSEINPUTBUTTON_P_H
#define PALAPELI_MOUSEINPUTBUTTON_P_H

#include <QLabel>
#include <QMouseEvent>

namespace Palapeli
{
	class FlatButton : public QLabel
	{
		Q_OBJECT
		public:
			FlatButton(const QIcon& icon, QWidget* parent = 0) : QLabel(parent), m_icon(icon)
			{
				leaveEvent(0); //apply icon
				setMouseTracking(true);
			}
		Q_SIGNALS:
			void clicked();
		protected:
			virtual void enterEvent(QEvent*)
			{
				//TODO: respect global icon size configuration
				setPixmap(m_icon.pixmap(22, QIcon::Active));
			}
			virtual void leaveEvent(QEvent*)
			{
				setPixmap(m_icon.pixmap(22, QIcon::Normal));
			}
			virtual void mousePressEvent(QMouseEvent* event)
			{
				if (event->button() == Qt::LeftButton)
					event->accept();
				else
					QLabel::mousePressEvent(event);
			}
			virtual void mouseReleaseEvent(QMouseEvent* event)
			{
				if (event->button() == Qt::LeftButton)
				{
					event->accept();
					if (rect().contains(event->pos()))
						emit clicked();
				}
				else
					QLabel::mouseReleaseEvent(event);
			}
		private:
			QIcon m_icon;
	};
}

#endif // PALAPELI_MOUSEINPUTBUTTON_P_H

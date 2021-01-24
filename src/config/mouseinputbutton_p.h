/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

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
			explicit FlatButton(const QIcon& icon, QWidget* parent = nullptr) : QLabel(parent), m_icon(icon)
			{
				leaveEvent(nullptr); //apply icon
				setMouseTracking(true);
			}
		Q_SIGNALS:
			void clicked();
		protected:
			void enterEvent(QEvent*) override
			{
				//TODO: respect global icon size configuration
				setPixmap(m_icon.pixmap(16, QIcon::Active));
			}
			void leaveEvent(QEvent*) override
			{
				setPixmap(m_icon.pixmap(16, QIcon::Normal));
			}
			void mousePressEvent(QMouseEvent* event) override
			{
				if (event->button() == Qt::LeftButton)
					event->accept();
				else
					QLabel::mousePressEvent(event);
			}
			void mouseReleaseEvent(QMouseEvent* event) override
			{
				if (event->button() == Qt::LeftButton)
				{
					event->accept();
					if (rect().contains(event->pos()))
						Q_EMIT clicked();
				}
				else
					QLabel::mouseReleaseEvent(event);
			}
		private:
			QIcon m_icon;
	};
}

#endif // PALAPELI_MOUSEINPUTBUTTON_P_H

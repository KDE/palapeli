/*
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
			explicit ZoomWidget(QWidget* parent = nullptr);
		public Q_SLOTS:
			void setConstrained(bool constrained);
			void setLevel(int level);
		Q_SIGNALS:
			void constrainedChanged(bool constrained);
			void levelChanged(int level);
			void zoomInRequest();
			void zoomOutRequest();
		private:
			QToolButton* m_constrainedButton;
			QToolButton* m_zoomOutButton;
			QToolButton* m_zoomInButton;
			QSlider* m_slider;
	};
}

#endif // PALAPELI_ZOOMWIDGET_H

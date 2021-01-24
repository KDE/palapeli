/*
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

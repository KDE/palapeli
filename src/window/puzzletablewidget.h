/*
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELI_PUZZLETABLEWIDGET_H
#define PALAPELI_PUZZLETABLEWIDGET_H

#include <QWidget>

class QStackedWidget;

namespace Palapeli
{
	class LoadingWidget;
	class TextProgressBar;
	class View;
	class ZoomWidget;

	class PuzzleTableWidget : public QWidget
	{
		Q_OBJECT
		public:
			PuzzleTableWidget();

			Palapeli::View* view() const;
		public Q_SLOTS:
			void reportProgress(int pieceCount, int partCount);
			void showStatusBar(bool visible);
		private Q_SLOTS:
			void setZoomAdjustable(bool adjustable);
		private:
			bool m_zoomAdjustable;
			QStackedWidget* m_stack;
			Palapeli::LoadingWidget* m_loadingWidget;
			Palapeli::View* m_view;
			Palapeli::TextProgressBar* m_progressBar;
			Palapeli::ZoomWidget* m_zoomWidget;
	};
}

#endif // PALAPELI_PUZZLETABLE_H

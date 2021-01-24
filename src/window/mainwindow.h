/*
    SPDX-FileCopyrightText: 2009-2011 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELI_MAINWINDOW_H
#define PALAPELI_MAINWINDOW_H

#include <KXmlGuiWindow>

namespace Palapeli
{
	class GamePlay;
	class Puzzle;

	class MainWindow : public KXmlGuiWindow
	{
		Q_OBJECT
		public:
            explicit MainWindow(const QString &path);
		protected:
			bool queryClose() override;
		private Q_SLOTS:
			void enableMessages();
		private:
			void setupActions();

			Palapeli::GamePlay* m_game;
	};
}

#endif // PALAPELI_MAINWINDOW_H

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

#ifndef PALAPELI_MAINWINDOW_H
#define PALAPELI_MAINWINDOW_H

class QDockWidget;
class KAction;
class KDialog;
#include <KXmlGuiWindow>

namespace Ui
{
	class NewPuzzleDialog;
	class SettingsWidget;
};

namespace Palapeli
{

	class ListMenu;
	class SaveAction;
	class SavegameView;

	class MainWindow : public KXmlGuiWindow
	{
		Q_OBJECT
		public:
			MainWindow(QWidget* parent = 0);
			~MainWindow();
		private Q_SLOTS:
			void setupDialogs();
			void configurationChanged();
			void configurationFinished();
			void startGame();
		private:
			KDialog* m_newDialog;
			Ui::NewPuzzleDialog* m_newUi;
			ListMenu* m_loadAct;
			SaveAction* m_saveAct;
			QDockWidget* m_dockMinimap;
			KAction* m_toggleMinimapAct;
			QDockWidget* m_dockPreview;
			KAction* m_togglePreviewAct;
			QDockWidget* m_dockSavegames;
			KAction* m_showSavegamesAct;
			KDialog* m_settingsDialog;
			Ui::SettingsWidget* m_settingsUi;
	};

}

#endif //PALAPELI_MAINWINDOW_H

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
};

namespace Palapeli
{

	class LoadAction;
	class Manager;
	class SaveAction;
	class SavegameView;

	class MainWindow : public KXmlGuiWindow
	{
		Q_OBJECT
		public:
			MainWindow(Manager* manager, QWidget* parent = 0);
			~MainWindow();
		private Q_SLOTS:
			void setupDialogs();
			void startGame();
			void loadGame();
			void saveGame();
		private:
			Manager* m_manager;
			LoadAction* m_loadAct;
			SaveAction* m_saveAct;
			QDockWidget* m_dockMinimap;
			KAction* m_toggleMinimapAct;
			QDockWidget* m_dockPreview;
			KAction* m_togglePreviewAct;
			QDockWidget* m_dockSavegames;
			KAction* m_showSavegamesAct;
			KDialog* m_newDialog;
			Ui::NewPuzzleDialog* m_newUi;
	};

}

#endif //PALAPELI_MAINWINDOW_H

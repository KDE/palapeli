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

#ifndef PALAPELI_MAINWINDOW_PRIVATE_H
#define PALAPELI_MAINWINDOW_PRIVATE_H

#include "mainwindow.h"

#include "listmenu.h"
#include "saveaction.h"
#include "savegameview.h"
#include "ui_dialognew.h"
#include "ui_settings.h"

#include <QDockWidget>
#include <KAction>
#include <KDialog>

namespace Palapeli
{

	class MainWindowPrivate : public QObject
	{
		Q_OBJECT
		public:
			MainWindowPrivate(MainWindow* parent);
			~MainWindowPrivate();
			MainWindow* m_parent;
		public Q_SLOTS:
			//setup functions
			void setupActions();
			void setupDockers();
			void setupDialogs();
			//event handlers
			void configurationChanged();
			void configurationFinished();
			void startGame();
		public:
			//actions
			ListMenu* m_loadAct;
			SaveAction* m_saveAct;
			KAction* m_showSavegamesAct;
			KAction* m_toggleMinimapAct;
			KAction* m_togglePreviewAct;
			//dockers
			QDockWidget* m_dockMinimap;
			QDockWidget* m_dockPreview;
			SavegameView* m_savegameView;
			QDockWidget* m_dockSavegames;
			//dialogs
			KDialog* m_newDialog;
			Ui::NewPuzzleDialog* m_newUi;
			KDialog* m_settingsDialog;
			Ui::SettingsWidget* m_settingsUi;
	};

}

#endif // PALAPELI_MAINWINDOW_PRIVATE_H

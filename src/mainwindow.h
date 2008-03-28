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

#include <KXmlGuiWindow>

class QDockWidget;
#include <QWidget>

namespace Palapeli
{
	class Minimap;
	class Preview;
	class View;
	
	class MainWindow : public KXmlGuiWindow
	{
		Q_OBJECT
		public:
			MainWindow(int sceneWidth, int sceneHeight, const QString &fileName, int xPieces, int yPieces, QWidget* parent = 0);
			~MainWindow();
			
		public Q_SLOTS:
			void startGame();
		
		private:
			QDockWidget* m_dockmap;
			Minimap* m_minimap;
			QDockWidget* m_dockpreview;
			Preview* m_preview;
			int m_sceneWidth;
			int m_sceneHeight;
			QString m_fileName;
			int m_xPieces;
			int m_yPieces;
			View* m_view;
	};
}

#endif //PALAPELI_MAINWINDOW_H
 

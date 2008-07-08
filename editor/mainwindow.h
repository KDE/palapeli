/***************************************************************************
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

#ifndef PALADESIGN_MAINWINDOW_H
#define PALADESIGN_MAINWINDOW_H

class QDockWidget;
class QTableView;
class KAction;
#include <KXmlGuiWindow>

namespace Paladesign
{

	class AddRelationAction;
	class Manager;
	class RemoveRelationAction;

	class MainWindow : public KXmlGuiWindow
	{
		Q_OBJECT
		public:
			MainWindow(Manager* manager);
			~MainWindow();
		protected Q_SLOTS:
			void selectShape();
		private:
			Manager* m_manager;
			QDockWidget* m_objectDock;
			KAction* m_toggleObjectDockAct;
			QDockWidget* m_propertyDock;
			KAction* m_togglePropertyDockAct;
			QTableView* m_propertyView;
			KAction* m_selectShapeAct;
			AddRelationAction* m_addRelationAct;
			RemoveRelationAction* m_removeRelationAct;
	};

}

#endif // PALADESIGN_MAINWINDOW_H

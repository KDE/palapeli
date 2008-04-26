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

#ifndef PALAPELI_VIEW_H
#define PALAPELI_VIEW_H

class QGraphicsScene;
#include <QGraphicsView>
class QWheelEvent;

namespace Palapeli
{

	class Manager;

	class View : public QGraphicsView
	{
		public:
			View(Manager* manager, QWidget* parent = 0);
		protected:
			virtual void wheelEvent(QWheelEvent* event);
		private:
			Manager* m_manager;
			QGraphicsScene* m_scene;
	};

}

#endif //PALAPELI_VIEW_H

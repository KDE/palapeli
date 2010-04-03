/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef PALAPELI_INTERACTORMANAGER_H
#define PALAPELI_INTERACTORMANAGER_H

class QGraphicsView;
#include <QMap>
class QMouseEvent;
#include <QObject>
class QWheelEvent;

namespace Palapeli
{
	class Interactor;

	class InteractorManager : public QObject
	{
		public:
			InteractorManager(QGraphicsView* view);

			bool handleMouseEvent(QMouseEvent* event);
			bool handleWheelEvent(QWheelEvent* event);
		private:
			QMap<QByteArray, QList<int> > m_defaultConfiguration;
			QMap<QByteArray, Palapeli::Interactor*> m_interactors;
	};
}

#endif // PALAPELI_INTERACTORMANAGER_H

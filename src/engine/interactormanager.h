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

#include <QGraphicsView>
#include <QMap>

namespace Palapeli
{
	class EventContext;
	class Interactor;
	class MouseEvent;

	class InteractorManager : public QObject
	{
		Q_OBJECT
		public:
			explicit InteractorManager(QGraphicsView* view);

			void handleEvent(QWheelEvent* event);
			void handleEvent(QMouseEvent* event);
			void handleEvent(QKeyEvent* event);

			void updateScene();
		public Q_SLOTS:
			void resetActiveTriggers();
		protected:
			void handleEventCommon(const Palapeli::MouseEvent& pEvent, QMap<Palapeli::Interactor*, Palapeli::EventContext>& interactorData, Qt::MouseButtons unhandledButtons);
		private:
			QGraphicsView* m_view;
			QMap<QByteArray, Palapeli::Interactor*> m_interactors; //NOTE: The interactor list is always hard-coded, based on what is available. The keys are used for writing the trigger list to the config.
			//state
			Qt::MouseButtons m_buttons;
			QPoint m_mousePos;
	};
}

#endif // PALAPELI_INTERACTORMANAGER_H

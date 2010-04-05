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

#include "interactorutils.h"

#include <QEvent>
#include <QGraphicsView>
#include <QMap>

namespace Palapeli
{
	class Interactor;
	typedef QPair<Palapeli::InteractorTrigger, Palapeli::Interactor*> AssociatedInteractorTrigger;

	class InteractorManager
	{
		public:
			explicit InteractorManager(QGraphicsView* view);

			bool handleEvent(QWheelEvent* event);
			bool handleEvent(QMouseEvent* event);
			bool handleEvent(QKeyEvent* event);

			void updateScene();
		protected:
			bool testTrigger(const Palapeli::InteractorTrigger& trigger, QWheelEvent* event);
			bool testTrigger(const Palapeli::InteractorTrigger& trigger, QMouseEvent* event);
			bool testTrigger(const Palapeli::InteractorTrigger& trigger, QKeyEvent* event);

			void handleMouseEvent(const Palapeli::MouseEvent& pEvent, QList<Palapeli::AssociatedInteractorTrigger>& matchingTriggers, QMap<Qt::MouseButton, QEvent::Type>& unhandledButtons);
		private:
			QGraphicsView* m_view;
			QMap<QByteArray, Palapeli::Interactor*> m_interactors;
			//configuration
			QList<Palapeli::AssociatedInteractorTrigger> m_triggers;
			//state
			QList<Palapeli::AssociatedInteractorTrigger> m_activeTriggers;
			Qt::MouseButtons m_buttons;
			QPoint m_mousePos;
			//quasi-static data
			QMap<Qt::Key, Qt::KeyboardModifier> m_keyModifierMap;
	};
}

#endif // PALAPELI_INTERACTORMANAGER_H

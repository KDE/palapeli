/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELI_INTERACTORMANAGER_H
#define PALAPELI_INTERACTORMANAGER_H

#include <QGraphicsView>
#include <QMap>

namespace Palapeli
{
	struct EventContext;
	class Interactor;
	struct MouseEvent;

	class InteractorManager : public QObject
	{
		Q_OBJECT
		public:
			explicit InteractorManager(QGraphicsView* view);
			~InteractorManager() override;

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

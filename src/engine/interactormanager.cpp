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

#include "interactormanager.h"
#include "triggermapper.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>

static const int AdditionalPriorityForExactMatches = 10000;

Palapeli::InteractorManager::InteractorManager(QGraphicsView* view)
	: QObject(view)
	, m_view(view)
	, m_interactors(Palapeli::TriggerMapper::createInteractors(view))
{
	connect(Palapeli::TriggerMapper::instance(), SIGNAL(associationsChanged()), SLOT(resetActiveTriggers()));
}

Palapeli::InteractorManager::~InteractorManager()
{
	qDeleteAll(m_interactors);
}

void Palapeli::InteractorManager::updateScene()
{
	foreach (Palapeli::Interactor* interactor, m_interactors)
		interactor->updateScene();
}

void Palapeli::InteractorManager::resetActiveTriggers()
{
	foreach (Palapeli::Interactor* interactor, m_interactors)
		interactor->setInactive();
}

/*
 * Wheel events are delivered to all interactors that accept them.
 */
void Palapeli::InteractorManager::handleEvent(QWheelEvent* event)
{
	//convert event
	Palapeli::WheelEvent pEvent(m_view, event->pos(), event->delta());
	//check which interactors are triggered by this event
	Palapeli::Interactor* bestMatchInteractor = 0;
	int bestMatchPriority = -1;
	QMap<QByteArray, Palapeli::Interactor*>::const_iterator it1 = m_interactors.constBegin(), it2 = m_interactors.constEnd();
	for (; it1 != it2; ++it1)
	{
		Palapeli::Interactor* const interactor = it1.value();
		const Palapeli::EventProcessingFlags flags = Palapeli::TriggerMapper::instance()->testTrigger(it1.key(), event);
		if (!(flags & Palapeli::EventMatches))
			continue;
		int priority = interactor->priority();
		if ((flags & Palapeli::EventMatchesExactly) == Palapeli::EventMatchesExactly)
			priority += AdditionalPriorityForExactMatches;
		if (priority > bestMatchPriority)
		{
			bestMatchInteractor = interactor;
			bestMatchPriority = priority;
		}
	}
	//activate matching interactor with highest priority
	if (bestMatchInteractor)
		bestMatchInteractor->sendEvent(pEvent);
}

/*
 * Unlike wheel events, mouse events are not just delivered to all interactors
 * that may accept them. Mouse interactions usually consist of a sequence of
 * press-move-move-...-release events, and we deliver all events of one sequence
 * to exactly one interactor. The Interactor class manages the activity flag
 * involved in this operation, and completes incomplete event sequences.
 */
void Palapeli::InteractorManager::handleEvent(QMouseEvent* event)
{
	//convert event
	Palapeli::MouseEvent pEvent(m_view, event->pos());
	//save button state (this information is needed for key events *following* this event, but not available from them)
	m_buttons = event->buttons();
	if (event->type() != QEvent::MouseButtonRelease)
		m_buttons |= event->button();
	m_mousePos = event->pos();
	//check which interactors are triggered by this event
	QMap<Palapeli::Interactor*, Palapeli::EventContext> interactorData;
	QMap<QByteArray, Palapeli::Interactor*>::const_iterator it1 = m_interactors.constBegin(), it2 = m_interactors.constEnd();
	for (; it1 != it2; ++it1)
		interactorData[it1.value()] = Palapeli::TriggerMapper::instance()->testTrigger(it1.key(), event);
	//further processing in a method which is shared with the KeyEvent handler
	handleEventCommon(pEvent, interactorData, event->buttons() | event->button());
}

/*
 * We also need to process KeyPress and KeyRelease events for modifier changes.
 */
void Palapeli::InteractorManager::handleEvent(QKeyEvent* event)
{
	//convert event
	Palapeli::MouseEvent pEvent(m_view, m_mousePos);
	//check which interactors are triggered by this event
	QMap<Palapeli::Interactor*, Palapeli::EventContext> interactorData;
	QMap<QByteArray, Palapeli::Interactor*>::const_iterator it1 = m_interactors.constBegin(), it2 = m_interactors.constEnd();
	for (; it1 != it2; ++it1)
		interactorData[it1.value()] = Palapeli::TriggerMapper::instance()->testTrigger(it1.key(), event, m_buttons);
	//further processing in a method which is shared with the MouseEvent handler
	handleEventCommon(pEvent, interactorData, m_buttons);
}

/*
 * This is the common base for handleEvent(QMouseEvent*) and handleEvent(QKeyEvent*).
 */
void Palapeli::InteractorManager::handleEventCommon(const Palapeli::MouseEvent& pEvent, QMap<Palapeli::Interactor*, Palapeli::EventContext>& interactorData, Qt::MouseButtons unhandledButtons)
{
	//try to use active triggers where possible
	foreach (Palapeli::Interactor* interactor, m_interactors)
		if (interactor->isActive())
		{
			//fetch flags, and remove them to mark this interactor as processed
			EventContext context = interactorData.value(interactor);
			interactorData.remove(interactor);
			//send event, mark button as processed
			if ((unhandledButtons & context.triggeringButtons) || context.triggeringButtons == Qt::NoButton)
			{
				interactor->sendEvent(pEvent, context.flags);
				if (interactor->isActive())
					unhandledButtons &= ~context.triggeringButtons;
			}
		}
	//sort remaining interactors by priority (the sorting is done by QMap)
	QMap<int, Palapeli::Interactor*> sortedInteractors;
	QMapIterator<Palapeli::Interactor*, EventContext> iter1(interactorData);
	while (iter1.hasNext())
	{
		Palapeli::Interactor* interactor = iter1.next().key();
		int priority = interactor->priority();
		if ((iter1.value().flags & Palapeli::EventMatchesExactly) == Palapeli::EventMatchesExactly)
			priority += AdditionalPriorityForExactMatches;
		//NOTE: The minus below implements a descending sort order.
		sortedInteractors.insertMulti(-priority, interactor);
	}
	//try to activate interactors with matching triggers
	foreach (Palapeli::Interactor* interactor, sortedInteractors)
	{
		const EventContext context = interactorData.value(interactor);
		//send event, mark button as processed
		if ((unhandledButtons & context.triggeringButtons) || context.triggeringButtons == Qt::NoButton)
		{
			interactor->sendEvent(pEvent, context.flags);
			if (interactor->isActive())
				unhandledButtons &= ~context.triggeringButtons;
		}
		else
			interactor->setInactive();
	}
}

#include "interactormanager.moc"

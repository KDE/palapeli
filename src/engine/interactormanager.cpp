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
#include "constraintinteractor.h"
#include "interactors.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>

Palapeli::InteractorManager::InteractorManager(QGraphicsView* view)
	: QObject(view)
	, m_view(view)
{
	//create interactors
	m_interactors["MovePiece"] = new Palapeli::MovePieceInteractor(view);
	m_interactors["SelectPiece"] = new Palapeli::SelectPieceInteractor(view);
	m_interactors["MoveViewport"] = new Palapeli::MoveViewportInteractor(view);
	m_interactors["ZoomViewport"] = new Palapeli::ZoomViewportInteractor(view);
	m_interactors["RubberBand"] = new Palapeli::RubberBandInteractor(view);
	m_interactors["Constraints"] = new Palapeli::ConstraintInteractor(view);
	//setup triggers (WARNING: the insertion order implements priority)
	typedef Palapeli::InteractorTrigger PIT;
	m_triggers << qMakePair(PIT("LeftButton;NoModifier"), m_interactors["MovePiece"]);
	m_triggers << qMakePair(PIT("LeftButton;ControlModifier"), m_interactors["SelectPiece"]);
	m_triggers << qMakePair(PIT("RightButton;NoModifier"), m_interactors["MoveViewport"]);
	m_triggers << qMakePair(PIT("wheel:Vertical;NoModifier"), m_interactors["ZoomViewport"]);
	m_triggers << qMakePair(PIT("LeftButton;NoModifier"), m_interactors["Constraints"]);
	m_triggers << qMakePair(PIT("LeftButton;NoModifier"), m_interactors["RubberBand"]);
	//initialize quasi-static data
	m_keyModifierMap[Qt::Key_Shift] = Qt::ShiftModifier;
	m_keyModifierMap[Qt::Key_Control] = Qt::ControlModifier;
	m_keyModifierMap[Qt::Key_Alt] = Qt::AltModifier;
	m_keyModifierMap[Qt::Key_Meta] = Qt::MetaModifier;
}

void Palapeli::InteractorManager::updateScene()
{
	foreach (Palapeli::Interactor* interactor, m_interactors)
		interactor->updateScene();
}

/*
 * Wheel events are delivered to all interactors that accept them.
 */
void Palapeli::InteractorManager::handleEvent(QWheelEvent* event)
{
	//convert event
	Palapeli::WheelEvent pEvent(m_view, event->pos(), event->delta());
	//try to handle event
	foreach (const Palapeli::AssociatedInteractorTrigger& aTrigger, m_triggers)
		if (testTrigger(aTrigger.first, event) & Palapeli::EventMatches)
			aTrigger.second->sendEvent(pEvent);
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
	//check which triggers are activated by this event
	QMap<Palapeli::Interactor*, EventContext> interactorData;
	foreach (const Palapeli::AssociatedInteractorTrigger& trigger, m_triggers)
	{
		//NOTE: One interactor may have multiple triggers, so we OR stuff together.
		const Palapeli::EventProcessingFlags flags = testTrigger(trigger.first, event);
		interactorData[trigger.second].flags |= flags;
		if (flags & Palapeli::EventMatches)
			interactorData[trigger.second].triggeringButtons |= trigger.first.button();
	}
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
	//check which triggers are activated by this event
	QMap<Palapeli::Interactor*, EventContext> interactorData;
	foreach (const Palapeli::AssociatedInteractorTrigger& trigger, m_triggers)
	{
		//NOTE: One interactor may have multiple triggers, so we OR all flags together.
		interactorData[trigger.second].flags |= testTrigger(trigger.first, event);
		interactorData[trigger.second].triggeringButtons |= trigger.first.button();
	}
	//further processing in a method which is shared with the KeyEvent handler
	handleEventCommon(pEvent, interactorData, m_buttons);
}

/*
 * This is the common base for handleEvent(QMouseEvent*) and handleEvent(QKeyEvent*).
 */
void Palapeli::InteractorManager::handleEventCommon(const Palapeli::MouseEvent& pEvent, QMap<Palapeli::Interactor*, EventContext>& interactorData, Qt::MouseButtons unhandledButtons)
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
	//try to activate interactors with matching triggers
	QMutableMapIterator<Palapeli::Interactor*, EventContext> iter(interactorData);
	while (iter.hasNext())
	{
		Palapeli::Interactor* interactor = iter.next().key();
		const EventContext context = iter.value();
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

Palapeli::EventProcessingFlags Palapeli::InteractorManager::testTrigger(const Palapeli::InteractorTrigger& trigger, QWheelEvent* event)
{
	if (trigger.isValid())
	{
		const bool testModifiers = trigger.modifiers() == event->modifiers();
		const bool checkDirection = trigger.wheelDirection() != 0;
		const bool testDirection = trigger.wheelDirection() == event->orientation();
		if (testModifiers && checkDirection && testDirection)
			return Palapeli::EventMatches;
	}
	//if execution comes to this point, trigger does not match
	return 0;
}

Palapeli::EventProcessingFlags Palapeli::InteractorManager::testTrigger(const Palapeli::InteractorTrigger& trigger, QMouseEvent* event)
{
	if (trigger.isValid())
	{
		const bool testModifiers = trigger.modifiers() == event->modifiers();
		const bool checkDirection = trigger.wheelDirection() == 0;
		if (testModifiers && checkDirection)
		{
			if (trigger.button() == Qt::NoButton)
				//trigger matches
				return Palapeli::EventMatches;
			const bool checkButtons = (event->button() | event->buttons()) & trigger.button();
			if (checkButtons)
			{
				//trigger matches - construct result
				Palapeli::EventProcessingFlags result = Palapeli::EventMatches;
				if (event->button() == trigger.button())
				{
					if (event->type() == QEvent::MouseButtonPress)
						result |= Palapeli::EventStartsInteraction;
					if (event->type() == QEvent::MouseButtonRelease)
						result |= Palapeli::EventConcludesInteraction;
				}
				return result;
			}
		}
	}
	//if execution comes to this point, trigger does not match
	return 0;
}

Palapeli::EventProcessingFlags Palapeli::InteractorManager::testTrigger(const Palapeli::InteractorTrigger& trigger, QKeyEvent* event)
{
	if (trigger.isValid())
	{
		//read modifiers
		const Qt::KeyboardModifier keyModifier = m_keyModifierMap.value((Qt::Key) event->key(), Qt::NoModifier);
		const Qt::KeyboardModifiers modifiers = keyModifier | event->modifiers();
		//checking
		const bool testModifiers = trigger.modifiers() == modifiers;
		const bool checkDirection = trigger.wheelDirection() == 0;
		const bool checkButton = trigger.button() & m_buttons;
		if (testModifiers && checkDirection && checkButton)
		{
			//trigger matches - construct result
			Palapeli::EventProcessingFlags result = Palapeli::EventMatches;
			if (keyModifier != Qt::NoModifier)
			{
					if (event->type() == QEvent::KeyPress)
						result |= Palapeli::EventStartsInteraction;
					if (event->type() == QEvent::KeyRelease)
						result |= Palapeli::EventConcludesInteraction;
			}
			return result;
		}
	}
	//if execution comes to this point, trigger does not match
	return 0;
}

//BEGIN API to configuration UI

const QList<Palapeli::Interactor*> Palapeli::InteractorManager::interactors() const
{
	return m_interactors.values();
}

const QList<Palapeli::AssociatedInteractorTrigger> Palapeli::InteractorManager::triggers() const
{
	return m_triggers;
}

void Palapeli::InteractorManager::setTriggers(const QList<Palapeli::AssociatedInteractorTrigger>& triggers)
{
	foreach (Palapeli::Interactor* interactor, m_interactors)
		interactor->setInactive();
	m_triggers = triggers;
	//TODO: write triggers to config file
}

//END API to configuration UI

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
	: m_view(view)
{
	//create interactors
	m_interactors["MovePiece"] = new Palapeli::MovePieceInteractor(view);
	m_interactors["MoveViewport"] = new Palapeli::MoveViewportInteractor(view);
	m_interactors["ZoomViewport"] = new Palapeli::ZoomViewportInteractor(view);
	m_interactors["RubberBand"] = new Palapeli::RubberBandInteractor(view);
	m_interactors["Constraints"] = new Palapeli::ConstraintInteractor(view);
	//setup triggers (WARNING: the insertion order implements priority)
	typedef Palapeli::InteractorTrigger PIT;
	m_triggers << qMakePair(PIT("LeftButton;NoModifier"), m_interactors["MovePiece"]);
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
 * We start with the simple case: Wheel events are delivered to all interactors 
 * that accept them.
 */
void Palapeli::InteractorManager::handleEvent(QWheelEvent* event)
{
	//convert event
	Palapeli::WheelEvent pEvent(m_view, event->pos(), event->delta());
	//try to handle event
	foreach (const Palapeli::AssociatedInteractorTrigger& aTrigger, m_triggers)
	{
		if (!testTrigger(aTrigger.first, event))
			continue;
		if (aTrigger.second->handleEvent(pEvent))
			return;
	}
}

/*
 * Unlike wheel events, mouse events are not just delivered to all interactors
 * that may accept them. Mouse interactions usually consist of a series of
 * press-move-move-...-release events, and we deliver all events of one series
 * to exactly one interactor. The only exception is that events with multiple
 * buttons can be delivered to multiple interactors to handle all buttons.
 */
void Palapeli::InteractorManager::handleEvent(QMouseEvent* event)
{
	//determine all triggers which are activated by this event
	QList<Palapeli::AssociatedInteractorTrigger> matchingTriggers(m_triggers);
	for (int i = 0; i < matchingTriggers.size(); ++i)
		if (!testTrigger(matchingTriggers[i].first, event))
			matchingTriggers.removeAt(i--); //decrement because another trigger is at this index after the removeAt()
	//convert event
	Palapeli::MouseEvent pEvent(m_view, event->pos());
	//save button state (this information is needed for key events *following* this event, but not available from them)
	m_buttons = event->buttons();
	if (event->type() != QEvent::MouseButtonRelease)
		m_buttons |= event->button();
	m_mousePos = event->pos();
	//find all keys which need to be handled
	QMap<Qt::MouseButton, QEvent::Type> unhandledButtons;
	switch (event->type())
	{
		case QEvent::MouseButtonPress:
		case QEvent::MouseButtonRelease:
			unhandledButtons.insert(event->button(), event->type());
			//FALL THROUGH (We need to interpret press/release events as move events for all other buttons, in order to keep the active triggers.)
		case QEvent::MouseMove:
			foreach (Qt::MouseButton button, Palapeli::analyzeFlags(event->buttons()))
				if (!unhandledButtons.contains(button))
					unhandledButtons.insert(button, QEvent::MouseMove);
			unhandledButtons.insert(Qt::NoButton, QEvent::MouseMove);
			break;
		default:
			return;
	}
	//further processing in a method which is shared with the KeyEvent handler
	handleMouseEvent(pEvent, matchingTriggers, unhandledButtons);
}

/*
 * We also need to process KeyPress and KeyRelease events for all triggers with
 * NoButton and NoOrientation. The MousePress-MouseMove-...-MouseRelease event
 * series becomes KeyPress-MouseMove-...-KeyRelease in this case (where KeyPress
 * and KeyRelease are for modifiers).
 */
void Palapeli::InteractorManager::handleEvent(QKeyEvent* event)
{
	//determine all triggers which are activated by this event
	QList<Palapeli::AssociatedInteractorTrigger> matchingTriggers(m_triggers);
	for (int i = 0; i < matchingTriggers.size(); ++i)
		if (!testTrigger(matchingTriggers[i].first, event))
			matchingTriggers.removeAt(i--); //decrement because another trigger is at this index after the removeAt()
	//convert event
	Palapeli::MouseEvent pEvent(m_view, m_mousePos);
	//find all keys which need to be handled
	QMap<Qt::MouseButton, QEvent::Type> unhandledButtons;
	foreach (Qt::MouseButton button, Palapeli::analyzeFlags(m_buttons))
		unhandledButtons.insert(button, event->type());
	unhandledButtons.insert(Qt::NoButton, event->type());
	//further processing in a method which is shared with the MouseEvent handler
	handleMouseEvent(pEvent, matchingTriggers, unhandledButtons);
}

/*
 * This is the common base for handleEvent(QMouseEvent*) and handleEvent(QKeyEvent*).
 */
void Palapeli::InteractorManager::handleMouseEvent(const Palapeli::MouseEvent& pEvent, QList<Palapeli::AssociatedInteractorTrigger>& matchingTriggers, QMap<Qt::MouseButton, QEvent::Type>& unhandledButtons)
{
	//try to use active triggers where possible
	QList<Palapeli::AssociatedInteractorTrigger> activeTriggersCopy(m_activeTriggers);
	foreach (const Palapeli::AssociatedInteractorTrigger& activeTrigger, activeTriggersCopy)
	{
		const Qt::MouseButton triggerButton = activeTrigger.first.button();
		QEvent::Type eventType = unhandledButtons.value(triggerButton, QEvent::None);
		bool keepTrigger = matchingTriggers.contains(activeTrigger);
		if (keepTrigger && eventType != QEvent::None)
			//event matches active trigger -> deliver event
			keepTrigger = activeTrigger.second->handleEvent(pEvent, eventType);
		else
			keepTrigger = false;
		//check if event has been accepted
		if (keepTrigger)
		{
			if (triggerButton != Qt::NoButton)
				unhandledButtons.remove(triggerButton);
		}
		if (eventType == QEvent::MouseButtonRelease)
			m_activeTriggers.removeAll(activeTrigger);
		else if (!keepTrigger)
		{
			activeTrigger.second->handleEvent(pEvent, QEvent::MouseButtonRelease);
			m_activeTriggers.removeAll(activeTrigger);
		}
		matchingTriggers.removeAll(activeTrigger); //no need to check this trigger again below
	}
	//try to find matching triggers for those buttons that have not been handled by the active triggers, and make these triggers active
	foreach (const Palapeli::AssociatedInteractorTrigger& trigger, matchingTriggers)
	{
		const Qt::MouseButton triggerButton = trigger.first.button();
		QEvent::Type eventType = unhandledButtons.value(triggerButton, QEvent::None);
		if (eventType == QEvent::None)
			continue; //this trigger button has been handled by an active trigger
		//try to handle this event
		bool eventHandled = true;
		if (eventType == QEvent::MouseMove)
			//always start with mousePressEvent
			eventHandled = trigger.second->handleEvent(pEvent, QEvent::MouseButtonPress);
		if (eventHandled)
			eventHandled = trigger.second->handleEvent(pEvent, eventType);
		if (eventHandled)
		{
			if (triggerButton != Qt::NoButton)
				unhandledButtons.remove(triggerButton);
			m_activeTriggers << trigger;
		}
	}
}

bool Palapeli::InteractorManager::testTrigger(const Palapeli::InteractorTrigger& trigger, QWheelEvent* event)
{
	return trigger.modifiers() == event->modifiers()
		&& trigger.wheelDirection() == event->orientation()
		&& trigger.wheelDirection() != 0;
}

bool Palapeli::InteractorManager::testTrigger(const Palapeli::InteractorTrigger& trigger, QMouseEvent* event)
{
	if (trigger.wheelDirection() != 0 || trigger.modifiers() != event->modifiers())
		return false;
	if (trigger.button() == Qt::NoButton)
		return true;
	else if (event->type() == QEvent::MouseMove)
		return event->buttons() & trigger.button();
	else
		return (event->button() | event->buttons()) & trigger.button();
}

bool Palapeli::InteractorManager::testTrigger(const Palapeli::InteractorTrigger& trigger, QKeyEvent* event)
{
	//read modifiers
	Qt::KeyboardModifiers modifiers = event->modifiers();
	const Qt::Key key = (Qt::Key) event->key();
	if (m_keyModifierMap.contains(key))
		modifiers |= m_keyModifierMap[key];
	//checking
	return trigger.modifiers() == modifiers
		&& trigger.button() == Qt::NoButton
		&& trigger.wheelDirection() == 0;
}

/***************************************************************************
 *   Copyright 2009 Chani Armitage <chani@kde.org>
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
***************************************************************************/

#include "mouseinputbutton.h"

#include <QApplication>
#include <QKeyEvent>
#include <KIcon>
#include <KLocalizedString>

const QString Palapeli::MouseInputButton::DefaultText = i18n("Set Trigger...");
const QString Palapeli::MouseInputButton::DefaultToolTip = i18n("Click to change how an action is triggered");

Palapeli::MouseInputButton::MouseInputButton(QWidget* parent)
	: QPushButton(parent)
	, m_noButtonAllowed(true)
	, m_requiresValidation(false)
{
	qRegisterMetaType<Palapeli::InteractorTrigger>();
	connect(this, SIGNAL(clicked()), SLOT(captureTrigger()));
	setCheckable(true);
	setChecked(false);
	setIcon(KIcon("input-mouse"));
	setText(DefaultText);
	setToolTip(DefaultToolTip);
}

bool Palapeli::MouseInputButton::isNoButtonAllowed() const
{
	return m_noButtonAllowed;
}

void Palapeli::MouseInputButton::setNoButtonAllowed(bool noButtonAllowed)
{
	m_noButtonAllowed = noButtonAllowed;
}

bool Palapeli::MouseInputButton::requiresValidation() const
{
	return m_requiresValidation;
}

void Palapeli::MouseInputButton::setRequiresValidation(bool requiresValidation)
{
	m_requiresValidation = requiresValidation;
}

Palapeli::InteractorTrigger Palapeli::MouseInputButton::trigger() const
{
	return m_trigger;
}

void Palapeli::MouseInputButton::captureTrigger()
{
	setChecked(true);
	setText(i18n("Input here..."));
	setToolTip(i18n("Hold down the modifier keys you want, then click a mouse button or scroll a mouse wheel here"));
	setFocus(Qt::MouseFocusReason);
}

void Palapeli::MouseInputButton::clearTrigger()
{
	setTrigger(Palapeli::InteractorTrigger());
}

bool Palapeli::MouseInputButton::event(QEvent* event)
{
	const QWheelEvent* wEvent = static_cast<QWheelEvent*>(event);
	const QMouseEvent* mEvent = static_cast<QMouseEvent*>(event);
	const QKeyEvent* kEvent = static_cast<QKeyEvent*>(event);
	if (isChecked())
	{
		//got a trigger or cancel
		switch ((int) event->type())
		{
			case QEvent::Wheel: {
				Palapeli::InteractorTrigger newTrigger;
				newTrigger.setModifiers(wEvent->modifiers());
				newTrigger.setWheelDirection(wEvent->orientation());
				setTrigger(newTrigger);
				event->accept();
				return true;
			}
			case QEvent::MouseButtonRelease: {
				Palapeli::InteractorTrigger newTrigger;
				newTrigger.setModifiers(mEvent->modifiers());
				newTrigger.setButton(mEvent->button());
				setTrigger(newTrigger);
				event->accept();
				return true;
			}
			case QEvent::MouseButtonPress:
				event->accept();
				return true;
			case QEvent::KeyPress: {
				if (kEvent->key() == Qt::Key_Escape)
				{
					//cancel
					setTrigger(m_trigger);
					event->accept();
					return true;
				}
				if (kEvent->key() == Qt::Key_Space && m_noButtonAllowed)
				{
					//create trigger with NoButton (TODO: make this functionality more user-visible)
					Palapeli::InteractorTrigger newTrigger;
					newTrigger.setModifiers(kEvent->modifiers());
					newTrigger.setButton(Qt::NoButton);
					setTrigger(newTrigger);
					event->accept();
					return true;
				}
			}	//fall through
			case QEvent::KeyRelease:
				showModifiers(kEvent->modifiers());
				break;
		}
	}
	bool ret = QPushButton::event(event);
	if (event->type() == QEvent::MouseButtonRelease)
	{
		//fake a tooltip event
		//because otherwise they go away when you click and don't come back until you move the mouse
		QHelpEvent tip(QEvent::ToolTip, mEvent->pos(), mEvent->globalPos());
		QApplication::sendEvent(this, &tip);
	}
	return ret;
}

void Palapeli::MouseInputButton::setTrigger(const Palapeli::InteractorTrigger& trigger)
{
	//NOTE: Invalid triggers need not be confirmed (esp. calls to clearTrigger()).
	if (m_requiresValidation && trigger.isValid() && m_trigger != trigger)
	{
		m_stagedTrigger = trigger;
		emit triggerRequest(trigger);
	}
	else
		applyTrigger(trigger);
}

void Palapeli::MouseInputButton::confirmTrigger(const Palapeli::InteractorTrigger& trigger)
{
	if (m_stagedTrigger == trigger)
		applyTrigger(m_stagedTrigger);
}

void Palapeli::MouseInputButton::applyTrigger(const Palapeli::InteractorTrigger& trigger)
{
	const bool announceChange = m_trigger != trigger;
	//apply new trigger
	m_trigger = trigger;
	m_stagedTrigger = Palapeli::InteractorTrigger();
	setChecked(false);
	setToolTip(DefaultToolTip);
	setText(m_trigger.isValid() ? m_trigger.toString() : DefaultText);
	//announce change
	if (announceChange)
		emit triggerChanged(trigger);
}

void Palapeli::MouseInputButton::showModifiers(Qt::KeyboardModifiers modifiers)
{
	Palapeli::InteractorTrigger dummyTrigger;
	dummyTrigger.setModifiers(modifiers);
	setText(dummyTrigger.toString().arg(i18n("Input here...")));
}

#include "mouseinputbutton.moc"

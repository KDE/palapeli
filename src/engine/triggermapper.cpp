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

#include "triggermapper.h"
#include "constraintinteractor.h"
#include "interactors.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <KConfig>
#include <KConfigGroup>
#include <KGlobal>

Palapeli::TriggerMapper* Palapeli::TriggerMapper::instance()
{
	static Palapeli::TriggerMapper instance;
	return &instance;
}

QMap<QByteArray, Palapeli::Interactor*> Palapeli::TriggerMapper::createInteractors(QGraphicsView* view)
{
	QMap<QByteArray, Palapeli::Interactor*> result;
	result["MovePiece"] = new Palapeli::MovePieceInteractor(view);
	result["SelectPiece"] = new Palapeli::SelectPieceInteractor(view);
	result["MoveViewport"] = new Palapeli::MoveViewportInteractor(view);
	result["ZoomViewport"] = new Palapeli::ZoomViewportInteractor(view);
	result["ScrollViewportHoriz"] = new Palapeli::ScrollViewportInteractor(Qt::Horizontal, view);
	result["ScrollViewportVert"] = new Palapeli::ScrollViewportInteractor(Qt::Vertical, view);
	result["RubberBand"] = new Palapeli::RubberBandInteractor(view);
	result["Constraints"] = new Palapeli::ConstraintInteractor(view);
	result["ToggleConstraints"] = new Palapeli::ToggleConstraintInteractor(view);
	return result;
}

QMap<QByteArray, Palapeli::Trigger> Palapeli::TriggerMapper::defaultAssociations()
{
	QMap<QByteArray, Palapeli::Trigger> result;
	result.insert("MovePiece", Palapeli::Trigger("LeftButton;NoModifier"));
	result.insert("SelectPiece", Palapeli::Trigger("LeftButton;ControlModifier"));
	result.insert("MoveViewport", Palapeli::Trigger("RightButton;NoModifier"));
	result.insert("ZoomViewport", Palapeli::Trigger("wheel:Vertical;NoModifier"));
	result.insert("RubberBand", Palapeli::Trigger("LeftButton;NoModifier"));
	result.insert("Constraints", Palapeli::Trigger("LeftButton;NoModifier"));
	return result;
}

Palapeli::TriggerMapper::TriggerMapper()
{
	//initialize quasi-static data
	m_keyModifierMap[Qt::Key_Shift] = Qt::ShiftModifier;
	m_keyModifierMap[Qt::Key_Control] = Qt::ControlModifier;
	m_keyModifierMap[Qt::Key_Alt] = Qt::AltModifier;
	m_keyModifierMap[Qt::Key_Meta] = Qt::MetaModifier;
	//initialize dynamic data
	readSettings();
}

QMap<QByteArray, Palapeli::Trigger> Palapeli::TriggerMapper::associations() const
{
	return m_associations;
}

void Palapeli::TriggerMapper::readSettings()
{
	m_associations.clear();
	//read config
	KConfigGroup group(KGlobal::config(), "Mouse Interaction");
	const QStringList configKeys = group.keyList();
	foreach (const QString& configKey, configKeys)
	{
		const QByteArray interactorKey = configKey.toLatin1();
		const QList<QByteArray> triggers = group.readEntry(configKey, QList<QByteArray>());
		foreach (const Palapeli::Trigger& trigger, triggers) //implicit casts FTW
			if (trigger.isValid())
				m_associations.insertMulti(interactorKey, trigger);
	}
	//fallback to default settings if necessary
	if (m_associations.isEmpty())
		m_associations = Palapeli::TriggerMapper::defaultAssociations();
	//announce update to InteractorManagers
	emit associationsChanged();
}

void Palapeli::TriggerMapper::setAssociations(const QMap<QByteArray, Palapeli::Trigger>& associations)
{
	m_associations = associations;
	//announce update to InteractorManagers
	emit associationsChanged();
	//assemble trigger serializations
	QMap<QByteArray, QList<QByteArray> > triggerSerializations;
	{
		QMap<QByteArray, Palapeli::Trigger>::const_iterator it1 = m_associations.constBegin(), it2 = m_associations.constEnd();
		for (; it1 != it2; ++it1)
			triggerSerializations[it1.key()] << it1.value().serialized();
	}
	//clear config
	KConfigGroup group(KGlobal::config(), "Mouse Interaction");
	foreach (const QString& key, group.keyList())
		group.deleteEntry(key);
	//write config (in a way that supports multiple triggers for one interactor)
	QMap<QByteArray, QList<QByteArray> >::const_iterator it1 = triggerSerializations.constBegin(), it2 = triggerSerializations.constEnd();
	for (; it1 != it2; ++it1)
		group.writeEntry(it1.key().data(), it1.value());
	KGlobal::config()->sync();
}

Palapeli::EventProcessingFlags Palapeli::TriggerMapper::testTrigger(const QByteArray& interactor, QWheelEvent* event) const
{
	Palapeli::EventProcessingFlags result;
	QMap<QByteArray, Palapeli::Trigger>::const_iterator it1 = m_associations.begin(), it2 = m_associations.end();
	for (; it1 != it2; ++it1)
		if (it1.key() == interactor)
			result |= testTrigger(it1.value(), event);
	return result;
}

Palapeli::EventContext Palapeli::TriggerMapper::testTrigger(const QByteArray& interactor, QMouseEvent* event) const
{
	Palapeli::EventContext result;
	QMap<QByteArray, Palapeli::Trigger>::const_iterator it1 = m_associations.begin(), it2 = m_associations.end();
	for (; it1 != it2; ++it1)
		if (it1.key() == interactor)
		{
			const Palapeli::EventProcessingFlags flags = testTrigger(it1.value(), event);
			result.flags |= flags;
			if (flags & Palapeli::EventMatches)
				result.triggeringButtons |= it1.value().button();
		}
	return result;
}

Palapeli::EventContext Palapeli::TriggerMapper::testTrigger(const QByteArray& interactor, QKeyEvent* event, Qt::MouseButtons buttons) const
{
	Palapeli::EventContext result;
	QMap<QByteArray, Palapeli::Trigger>::const_iterator it1 = m_associations.begin(), it2 = m_associations.end();
	for (; it1 != it2; ++it1)
		if (it1.key() == interactor)
		{
			result.flags |= testTrigger(it1.value(), event, buttons);
			result.triggeringButtons |= it1.value().button();
		}
	return result;
}

Palapeli::EventProcessingFlags Palapeli::TriggerMapper::testTrigger(const Palapeli::Trigger& trigger, QWheelEvent* event) const
{
	if (trigger.isValid())
	{
		const bool testModifiers = trigger.modifiers() == Qt::NoModifier || trigger.modifiers() == event->modifiers();
		const bool checkDirection = trigger.wheelDirection() != 0;
		const bool testDirection = trigger.wheelDirection() == event->orientation();
		if (testModifiers && checkDirection && testDirection)
		{
			if (trigger.modifiers() == event->modifiers())
				return Palapeli::EventMatchesExactly;
			else
				return Palapeli::EventMatches;
		}
	}
	//if execution comes to this point, trigger does not match
	return 0;
}

Palapeli::EventProcessingFlags Palapeli::TriggerMapper::testTrigger(const Palapeli::Trigger& trigger, QMouseEvent* event) const
{
	if (trigger.isValid())
	{
		const bool testModifiers = trigger.modifiers() == Qt::NoModifier || trigger.modifiers() == event->modifiers();
		const Palapeli::EventProcessingFlags positiveResult = (trigger.modifiers() == event->modifiers()) ? Palapeli::EventMatchesExactly : Palapeli::EventMatches;
		const bool checkDirection = trigger.wheelDirection() == 0;
		if (testModifiers && checkDirection)
		{
			if (trigger.button() == Qt::NoButton)
				//trigger matches
				return positiveResult;
			const bool checkButtons = (event->button() | event->buttons()) & trigger.button();
			if (checkButtons)
			{
				//trigger matches - construct result
				Palapeli::EventProcessingFlags result = positiveResult;
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

Palapeli::EventProcessingFlags Palapeli::TriggerMapper::testTrigger(const Palapeli::Trigger& trigger, QKeyEvent* event, Qt::MouseButtons buttons) const
{
	if (trigger.isValid())
	{
		//read modifiers
		const Qt::KeyboardModifier keyModifier = m_keyModifierMap.value((Qt::Key) event->key(), Qt::NoModifier);
		const Qt::KeyboardModifiers modifiers = keyModifier | event->modifiers();
		//checking
		const bool testModifiers = trigger.modifiers() == Qt::NoModifier || trigger.modifiers() == modifiers;
		const Palapeli::EventProcessingFlags positiveResult = trigger.modifiers() == event->modifiers() ? Palapeli::EventMatchesExactly : Palapeli::EventMatches;
		const bool checkDirection = trigger.wheelDirection() == 0;
		const bool checkButton = (trigger.button() & buttons) || trigger.button() == Qt::NoButton;
		if (testModifiers && checkDirection && checkButton)
		{
			//trigger matches - construct result
			Palapeli::EventProcessingFlags result = positiveResult;
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

#include "triggermapper.moc"

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

#include "interactorutils.h"

#include <QGraphicsView>

//BEGIN events

Palapeli::MouseEvent::MouseEvent(QGraphicsView* view, const QPoint& pos_)
	: pos(pos_)
	, scenePos(view->mapToScene(pos_))
{
}

Palapeli::WheelEvent::WheelEvent(QGraphicsView* view, const QPoint& pos_, int delta_)
	: pos(pos_)
	, scenePos(view->mapToScene(pos_))
	, delta(delta_)
{
}

//END events
//BEGIN Palapeli::InteractorTrigger

Palapeli::InteractorTrigger::InteractorTrigger()
{
}

Palapeli::InteractorTrigger::InteractorTrigger(const QString& serialization)
{
	//expect format "BUTTON_OR_WHEEL;MODIFIERLIST", i.e. two sections separated by semicolon
	const QStringList sections = serialization.split(';');
	if (sections.size() != 2)
		return;
	//parse first section ("BUTTON_OR_WHEEL")
	Qt::MouseButton button = Qt::NoButton;
	Qt::Orientation wheelDirection = (Qt::Orientation) 0;
	const QString section1 = sections.value(0);
	if (section1.startsWith(QLatin1String("wheel:")))
	{
		//parse wheel direction
		const QString wheelDirectionString = section1.mid(6);
		if (wheelDirectionString == QLatin1String("Horizontal"))
			wheelDirection = Qt::Horizontal;
		else if (wheelDirectionString == QLatin1String("Vertical"))
			wheelDirection = Qt::Vertical;
		else
			return; //parsing failed
	}
	else
	{
		//parse button
		if (section1 == QLatin1String("NoButton"))
			button = Qt::NoButton;
		else if (section1 == QLatin1String("LeftButton"))
			button = Qt::LeftButton;
		else if (section1 == QLatin1String("RightButton"))
			button = Qt::RightButton;
		else if (section1 == QLatin1String("MidButton"))
			button = Qt::MidButton;
		else if (section1 == QLatin1String("XButton1"))
			button = Qt::XButton1;
		else if (section1 == QLatin1String("XButton2"))
			button = Qt::XButton2;
		else
			return; //parsing failed
	}
	//parse modifier list (separated by vertical pipes)
	Qt::KeyboardModifiers modifiers;
	if (sections.value(1) != QLatin1String("NoModifier"))
	{
		const QStringList modifierStrings = sections.value(1).split('|');
		foreach (const QString& modifierString, modifierStrings)
		{
			if (modifierString == QLatin1String("ShiftModifier"))
				modifiers |= Qt::ShiftModifier;
			else if (modifierString == QLatin1String("ControlModifier"))
				modifiers |= Qt::ControlModifier;
			else if (modifierString == QLatin1String("AltModifier"))
				modifiers |= Qt::AltModifier;
			else if (modifierString == QLatin1String("MetaModifier"))
				modifiers |= Qt::MetaModifier;
			else if (modifierString == QLatin1String("GroupSwitchModifier"))
				modifiers |= Qt::GroupSwitchModifier;
			else
				return; //parsing failed
		}
	}
	//parsing succeeded
	m_modifiers = modifiers;
	m_button = button;
	m_wheelDirection = wheelDirection;
}

QString Palapeli::InteractorTrigger::serialized() const
{
	QString result;
	//write wheel direction
	if (m_wheelDirection == Qt::Horizontal)
		result = QLatin1String("wheel:Horizontal");
	else if (m_wheelDirection == Qt::Vertical)
		result = QLatin1String("wheel:Vertical");
	//write button
	else if (m_button == Qt::LeftButton)
		result = QLatin1String("LeftButton");
	else if (m_button == Qt::RightButton)
		result = QLatin1String("RightButton");
	else if (m_button == Qt::MidButton)
		result = QLatin1String("MidButton");
	else if (m_button == Qt::XButton1)
		result = QLatin1String("XButton1");
	else if (m_button == Qt::XButton2)
		result = QLatin1String("XButton2");
	else
		result = QLatin1String("NoButton");
	//write modifiers
	QChar separator = ';';
	if (m_modifiers & Qt::ShiftModifier)
	{
		result += separator + QLatin1String("ShiftModifier");
		separator = '|';
	}
	if (m_modifiers & Qt::ControlModifier)
	{
		result += separator + QLatin1String("ControlModifier");
		separator = '|';
	}
	if (m_modifiers & Qt::AltModifier)
	{
		result += separator + QLatin1String("AltModifier");
		separator = '|';
	}
	if (m_modifiers & Qt::MetaModifier)
	{
		result += separator + QLatin1String("MetaModifier");
		separator = '|';
	}
	if (m_modifiers & Qt::GroupSwitchModifier)
	{
		result += separator + QLatin1String("GroupSwitchModifier");
		separator = '|';
	}
	//if we wrote nothing, insert NoModifier
	if (separator == ';')
		result += separator + QLatin1String("NoModifier");
	return result;
}

Qt::KeyboardModifiers Palapeli::InteractorTrigger::modifiers() const
{
	return m_modifiers;
}

void Palapeli::InteractorTrigger::setModifiers(Qt::KeyboardModifiers modifiers)
{
	m_modifiers = modifiers;
}

Qt::MouseButton Palapeli::InteractorTrigger::button() const
{
	return m_button;
}

void Palapeli::InteractorTrigger::setButton(Qt::MouseButton button)
{
	m_button = button;
	if (m_button != Qt::NoButton)
		m_wheelDirection = (Qt::Orientation) 0;
}

Qt::Orientation Palapeli::InteractorTrigger::wheelDirection() const
{
	return m_wheelDirection;
}

void Palapeli::InteractorTrigger::setWheelDirection(Qt::Orientation orientation)
{
	m_wheelDirection = orientation;
	if (m_wheelDirection != (Qt::Orientation) 0)
		m_button = Qt::NoButton;
}

bool Palapeli::InteractorTrigger::operator==(const Palapeli::InteractorTrigger& other) const
{
	return m_modifiers == other.m_modifiers
		&& m_button == other.m_button
		&& m_wheelDirection == other.m_wheelDirection;
}

//END Palapeli::InteractorTrigger

QList<Qt::MouseButton> Palapeli::analyzeFlags(Qt::MouseButtons buttons)
{
	QList<Qt::MouseButton> result;
	if (buttons & Qt::LeftButton)
		result << Qt::LeftButton;
	if (buttons & Qt::RightButton)
		result << Qt::RightButton;
	if (buttons & Qt::MidButton)
		result << Qt::MidButton;
	if (buttons & Qt::XButton1)
		result << Qt::XButton1;
	if (buttons & Qt::XButton2)
		result << Qt::XButton2;
	return result;
}

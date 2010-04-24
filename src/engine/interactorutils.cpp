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
#include <QMap>
#include <KGlobal>
#include <KLocalizedString>

//BEGIN events

Palapeli::MouseEvent::MouseEvent(QGraphicsView* view, const QPoint& pos_)
	: pos(pos_)
	, scenePos(view->mapToScene(pos_))
{
}

Palapeli::MouseEvent::MouseEvent()
{
}

Palapeli::WheelEvent::WheelEvent(QGraphicsView* view, const QPoint& pos_, int delta_)
	: pos(pos_)
	, scenePos(view->mapToScene(pos_))
	, delta(delta_)
{
}

//END events

struct Palapeli::InteractorTrigger::Data
{
	QMap<Qt::KeyboardModifier, QString> m_modifierStrings;
	QMap<Qt::MouseButton, QString> m_buttonStrings;
	QMap<Qt::Orientation, QString> m_orientationStrings;

	Data(bool machineReadableStrings)
	{
		if (machineReadableStrings)
		{
			m_modifierStrings[Qt::ShiftModifier] = QLatin1String("ShiftModifier");
			m_modifierStrings[Qt::ControlModifier] = QLatin1String("ControlModifier");
			m_modifierStrings[Qt::AltModifier] = QLatin1String("AltModifier");
			m_modifierStrings[Qt::MetaModifier] = QLatin1String("MetaModifier");
			m_modifierStrings[Qt::GroupSwitchModifier] = QLatin1String("GroupSwitchModifier");
			m_buttonStrings[Qt::NoButton] = QLatin1String("NoButton");
			m_buttonStrings[Qt::LeftButton] = QLatin1String("LeftButton");
			m_buttonStrings[Qt::RightButton] = QLatin1String("RightButton");
			m_buttonStrings[Qt::MidButton] = QLatin1String("MidButton");
			m_buttonStrings[Qt::XButton1] = QLatin1String("XButton1");
			m_buttonStrings[Qt::XButton2] = QLatin1String("XButton2");
			m_orientationStrings[Qt::Horizontal] = QLatin1String("wheel:Horizontal");
			m_orientationStrings[Qt::Vertical] = QLatin1String("wheel:Vertical");
		}
		else
		{
			//FIXME: These are probably wrong for Macs.
			m_modifierStrings[Qt::ShiftModifier] = i18nc("a keyboard modifier", "Shift");
			m_modifierStrings[Qt::ControlModifier] = i18nc("a keyboard modifier", "Ctrl");
			m_modifierStrings[Qt::AltModifier] = i18nc("a keyboard modifier", "Alt");
			m_modifierStrings[Qt::MetaModifier] = i18nc("a keyboard modifier", "Meta");
			m_modifierStrings[Qt::GroupSwitchModifier] = i18nc("a special keyboard modifier", "GroupSwitch");
			m_buttonStrings[Qt::NoButton] = i18nc("refers to no mouse buttons being pressed", "No-Button");
			//FIXME: Left/right may be wrong if mouse buttons are swapped.
			m_buttonStrings[(Qt::MouseButton) -1] = QLatin1String("%1"); //allow external users to insert their custom strings into here
			m_buttonStrings[Qt::LeftButton] = i18nc("a mouse button", "Left-Button");
			m_buttonStrings[Qt::RightButton] = i18nc("a mouse button", "Right-Button");
			m_buttonStrings[Qt::MidButton] = i18nc("a mouse button", "Middle-Button");
			m_buttonStrings[Qt::XButton1] = i18nc("a special mouse button", "XButton1");
			m_buttonStrings[Qt::XButton2] = i18nc("a special mouse button", "XButton2");
			m_orientationStrings[Qt::Horizontal] = i18n("Horizontal-Scroll");
			m_orientationStrings[Qt::Vertical] = i18n("Vertical-Scroll");
		}
	}
};

K_GLOBAL_STATIC_WITH_ARGS(Palapeli::InteractorTrigger::Data, itParserData, (true))
K_GLOBAL_STATIC_WITH_ARGS(Palapeli::InteractorTrigger::Data, itPrettyData, (false))

//BEGIN Palapeli::InteractorTrigger

Palapeli::InteractorTrigger::InteractorTrigger()
	: m_button((Qt::MouseButton) -1)
{
}

Palapeli::InteractorTrigger::InteractorTrigger(const QString& serialization)
{
	const Palapeli::InteractorTrigger::Data& data = *itParserData;
	//expect format "BUTTON_OR_WHEEL;MODIFIERLIST", i.e. two sections separated by semicolon
	const QStringList sections = serialization.split(';');
	if (sections.size() != 2)
		return;
	//parse modifier list (separated by vertical pipes) -> We do this first because this one can fail.
	Qt::KeyboardModifiers modifiers = Qt::NoModifier;
	if (sections[1] != QLatin1String("NoModifier"))
	{
		const QStringList modifierStrings = sections[1].split('|');
		foreach (const QString& modifierString, modifierStrings)
		{
			Qt::KeyboardModifier modifier = data.m_modifierStrings.key(modifierString, Qt::NoModifier);
			if (modifier == Qt::NoModifier)
				return; //parsing failed
			modifiers |= modifier;
		}
	}
	m_modifiers = modifiers;
	//parse first section (button or wheel direction) -> default to NoButton + NoOrientation if parsing fails
	m_button = data.m_buttonStrings.key(sections[0], Qt::NoButton);
	m_wheelDirection = data.m_orientationStrings.key(sections[0], (Qt::Orientation) 0);
}

QPair<QString, QStringList> Palapeli::InteractorTrigger::toStringGeneric(const Palapeli::InteractorTrigger::Data& data) const
{
	//find modifier strings
	QStringList modifierStrings;
	QMap<Qt::KeyboardModifier, QString>::const_iterator it1 = data.m_modifierStrings.begin(), it2 = data.m_modifierStrings.end();
	for (; it1 != it2; ++it1)
		if (m_modifiers & it1.key())
			modifierStrings << it1.value();
	//write wheel direction or mouse button
	QString actionString;
	if (data.m_orientationStrings.contains(m_wheelDirection))
		actionString = data.m_orientationStrings[m_wheelDirection];
	else
		actionString = data.m_buttonStrings[m_button];
	//pass result bits to caller
	return qMakePair(actionString, modifierStrings);
}

bool Palapeli::InteractorTrigger::isValid() const
{
	if (m_wheelDirection > 0 && m_button > 0)
		return false; //do not allow wheel and mouse triggers at the same time
	else
		return m_button >= 0; //do not allow negative m_button values
}

QString Palapeli::InteractorTrigger::serialized() const
{
	if (!isValid())
		return QString();
	QPair<QString, QStringList> bits = toStringGeneric(*itParserData);
	if (bits.second.isEmpty())
		bits.second << QLatin1String("NoModifier");
	return bits.first + QChar(';') + bits.second.join(QChar('|'));
}

QString Palapeli::InteractorTrigger::toString() const
{
	const QPair<QString, QStringList> bits = toStringGeneric(*itPrettyData);
	if (bits.second.isEmpty())
		return bits.first;
	else
		return bits.second.join(QChar('+')) + '+' + bits.first;
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
	const QList<Qt::MouseButton>& knownButtons = itParserData->m_buttonStrings.keys();
	QList<Qt::MouseButton> result;
	foreach (Qt::MouseButton button, knownButtons)
		if (buttons & button)
			result << button;
	return result;
}

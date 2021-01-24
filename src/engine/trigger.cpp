/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "trigger.h"

#include <QMap>
#include <KLocalizedString>

namespace
{
	class ByteArrayList : public QList<QByteArray>
	{
		public:
			QByteArray join(const QChar& separator) const
			{
				char sep = separator.toLatin1();
				if (isEmpty())
					return QByteArray();
				int size = -1;
				for (int i = 0; i < count(); ++i)
					size += 1 + at(i).size();
				QByteArray result;
				result.reserve(size);
				for (int i = 0; i < count(); ++i)
				{
					if (i != 0)
						result.append(sep);
					result.append(at(i));
				}
				return result;
			}
	};

	template<typename T, typename ListType> struct TriggerData
	{
		QMap<Qt::KeyboardModifier, T> m_modifierStrings;
		QMap<Qt::MouseButton, T> m_buttonStrings;
		QMap<Qt::Orientation, T> m_orientationStrings;

		T modifierString(Qt::KeyboardModifiers modifiers, const QChar& separator) const
		{
			typename QMap<Qt::KeyboardModifier, T>::const_iterator it1 = m_modifierStrings.begin(), it2 = m_modifierStrings.end();
			ListType modifierStrings;
			for (; it1 != it2; ++it1)
				if (modifiers & it1.key())
					modifierStrings << it1.value();
			return modifierStrings.join(separator);
		}

		T actionString(Qt::Orientation wheelDirection, Qt::MouseButton button) const
		{
			if (m_orientationStrings.contains(wheelDirection))
				return m_orientationStrings.value(wheelDirection);
			else
				return m_buttonStrings.value(button);
		}
	};

	struct TriggerParserData : public TriggerData<QByteArray, ByteArrayList>
	{
		TriggerParserData()
		{
			m_modifierStrings[Qt::ShiftModifier] = "ShiftModifier";
			m_modifierStrings[Qt::ControlModifier] = "ControlModifier";
			m_modifierStrings[Qt::AltModifier] = "AltModifier";
			m_modifierStrings[Qt::MetaModifier] = "MetaModifier";
			m_modifierStrings[Qt::GroupSwitchModifier] = "GroupSwitchModifier";
			m_buttonStrings[Qt::NoButton] = "NoButton";
			m_buttonStrings[Qt::LeftButton] = "LeftButton";
			m_buttonStrings[Qt::RightButton] = "RightButton";
			m_buttonStrings[Qt::MiddleButton] = "MidButton";
			m_buttonStrings[Qt::XButton1] = "XButton1";
			m_buttonStrings[Qt::XButton2] = "XButton2";
			m_orientationStrings[Qt::Horizontal] = "wheel:Horizontal";
			m_orientationStrings[Qt::Vertical] = "wheel:Vertical";
		}
	};

	struct TriggerPrinterData : public TriggerData<QString, QStringList>
	{
		TriggerPrinterData()
		{
			//FIXME: These are probably wrong for Macs.
			m_modifierStrings[Qt::ShiftModifier] = i18nc("a keyboard modifier", "Shift");
			m_modifierStrings[Qt::ControlModifier] = i18nc("a keyboard modifier", "Ctrl");
			m_modifierStrings[Qt::AltModifier] = i18nc("a keyboard modifier", "Alt");
			m_modifierStrings[Qt::MetaModifier] = i18nc("a keyboard modifier", "Meta");
			m_modifierStrings[Qt::GroupSwitchModifier] = i18nc("a special keyboard modifier", "GroupSwitch");
			m_buttonStrings[Qt::NoButton] = i18nc("refers to no mouse buttons being pressed", "No-Button");
			//FIXME: Left/right may be wrong if mouse buttons are swapped.
			m_buttonStrings[(Qt::MouseButton) -1] = QStringLiteral("%1"); //allow external users to insert their custom strings into here
			m_buttonStrings[Qt::LeftButton] = i18nc("a mouse button", "Left-Button");
			m_buttonStrings[Qt::RightButton] = i18nc("a mouse button", "Right-Button");
			m_buttonStrings[Qt::MiddleButton] = i18nc("a mouse button", "Middle-Button");
			m_buttonStrings[Qt::XButton1] = i18nc("a special mouse button", "XButton1");
			m_buttonStrings[Qt::XButton2] = i18nc("a special mouse button", "XButton2");
			m_orientationStrings[Qt::Horizontal] = i18n("Horizontal-Scroll");
			m_orientationStrings[Qt::Vertical] = i18n("Vertical-Scroll");
		}
	};
}

Q_GLOBAL_STATIC(TriggerParserData, tParserData)
Q_GLOBAL_STATIC(TriggerPrinterData, tPrinterData)

Palapeli::Trigger::Trigger()
	: m_modifiers(Qt::NoModifier)
	, m_button((Qt::MouseButton) -1)
	, m_wheelDirection((Qt::Orientation) 0)
{
}

Palapeli::Trigger::Trigger(const QByteArray& serialization)
	: m_modifiers(Qt::NoModifier)
	, m_button((Qt::MouseButton) -1)
	, m_wheelDirection((Qt::Orientation) 0)
{
	//expect format "BUTTON_OR_WHEEL;MODIFIERLIST", i.e. two sections separated by semicolon
	const QList<QByteArray> sections = serialization.split(';');
	if (sections.size() != 2)
		return;
	//parse modifier list (separated by vertical pipes) -> We do this first because this one can fail.
	Qt::KeyboardModifiers modifiers = Qt::NoModifier;
	if (sections[1] != "NoModifier")
	{
		const QList<QByteArray> modifierStrings = sections[1].split('|');
		for (const QByteArray& modifierString : modifierStrings)
		{
			Qt::KeyboardModifier modifier = tParserData->m_modifierStrings.key(modifierString, Qt::NoModifier);
			if (modifier == Qt::NoModifier)
				return; //parsing failed
			modifiers |= modifier;
		}
	}
	m_modifiers = modifiers;
	//parse first section (button or wheel direction) -> default to NoButton + NoOrientation if parsing fails
	m_button = tParserData->m_buttonStrings.key(sections[0], Qt::NoButton);
	m_wheelDirection = tParserData->m_orientationStrings.key(sections[0], (Qt::Orientation) 0);
}

bool Palapeli::Trigger::isValid() const
{
	if (m_wheelDirection > 0 && m_button > 0)
		return false; //do not allow wheel and mouse triggers at the same time
	else
		return m_button != ((Qt::MouseButton) -1); //do not allow 'negative' m_button values
}

QByteArray Palapeli::Trigger::serialized() const
{
	if (!isValid())
		return QByteArray();
	const QByteArray actionString = tParserData->actionString(m_wheelDirection, m_button);
	QByteArray modifierString = tParserData->modifierString(m_modifiers, QLatin1Char('|'));
	if (modifierString.isEmpty())
		modifierString = "NoModifier";
	return actionString + QByteArray(1, ';') + modifierString;
}

QString Palapeli::Trigger::toString() const
{
	const QString actionString = tPrinterData->actionString(m_wheelDirection, m_button);
	const QString modifierString = tPrinterData->modifierString(m_modifiers, QLatin1Char('+'));
	if (modifierString.isEmpty())
		return actionString;
	else
		return modifierString + QLatin1Char('+') + actionString;
}

Qt::KeyboardModifiers Palapeli::Trigger::modifiers() const
{
	return m_modifiers;
}

void Palapeli::Trigger::setModifiers(Qt::KeyboardModifiers modifiers)
{
	m_modifiers = modifiers;
}

Qt::MouseButton Palapeli::Trigger::button() const
{
	return m_button;
}

void Palapeli::Trigger::setButton(Qt::MouseButton button)
{
	m_button = button;
	if (m_button != Qt::NoButton)
		m_wheelDirection = (Qt::Orientation) 0;
}

Qt::Orientation Palapeli::Trigger::wheelDirection() const
{
	return m_wheelDirection;
}

void Palapeli::Trigger::setWheelDirection(Qt::Orientation orientation)
{
	m_wheelDirection = orientation;
	if (m_wheelDirection != (Qt::Orientation) 0)
		m_button = Qt::NoButton;
}

bool Palapeli::Trigger::operator==(const Palapeli::Trigger& other) const
{
	return m_modifiers == other.m_modifiers
		&& m_button == other.m_button
		&& m_wheelDirection == other.m_wheelDirection;
}

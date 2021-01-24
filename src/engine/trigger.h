/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELI_TRIGGER_H
#define PALAPELI_TRIGGER_H

#include <QMetaType>
#include <QString>

namespace Palapeli
{
	struct Trigger
	{
		public:
			///Constructs an invalid trigger. (The mouse button is set to -1.)
			Trigger();
			///Constructs a trigger from the given serialization. If the parsing fails, this constructor returns an invalid trigger (just like the default constructor).
			///Possible serializations include "MidButton;NoModifier", "RightButton;ShiftModifier" and "wheel:Horizontal;ShiftModifier|ControlModifier". (A formal specification of the format is left as an exercise to the reader.)
			Trigger(const QByteArray& serialization); //krazy:exclude=explicit (I want implicit conversions)

			///Returns whether this triger is valid.
			bool isValid() const;
			///Returns the serialization for this trigger, or an empty string if this trigger is invalid.
			///\see isValid()
			QByteArray serialized() const;
			///Returns a translated (i.e. user-compatible) string representation for this trigger. This representation is not suitable for machine-readable files, use serialized() instead.
			QString toString() const;

			Qt::KeyboardModifiers modifiers() const;
			void setModifiers(Qt::KeyboardModifiers modifiers);
			Qt::MouseButton button() const; ///< Returns 0 by default or when a trigger wheel direction has been set.
			void setButton(Qt::MouseButton button); ///< Setting this will reset wheelDirection().
			Qt::Orientation wheelDirection() const; ///< Returns 0 by default or when a trigger button has been set.
			void setWheelDirection(Qt::Orientation orientation); ///< Setting this will reset button().

			bool operator==(const Palapeli::Trigger& other) const;
			inline bool operator!=(const Palapeli::Trigger& other) const;
		private:
			Qt::KeyboardModifiers m_modifiers;
			Qt::MouseButton m_button;
			Qt::Orientation m_wheelDirection;
	};
}

bool Palapeli::Trigger::operator!=(const Palapeli::Trigger& other) const
{
	return !(*this == other);
}

Q_DECLARE_METATYPE(Palapeli::Trigger)

#endif // PALAPELI_TRIGGER_H

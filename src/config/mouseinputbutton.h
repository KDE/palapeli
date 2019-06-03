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

#ifndef PALAPELI_MOUSEINPUTBUTTON_H
#define PALAPELI_MOUSEINPUTBUTTON_H

#include "../engine/trigger.h"

class QLabel;
#include <QPushButton>

namespace Palapeli
{
	class MouseInputButton : public QPushButton
	{
		Q_OBJECT
		public:
			explicit MouseInputButton(QWidget* parent = 0);
			QSize sizeHint() const Q_DECL_OVERRIDE;

			///If unset, this button will not generate any wheel triggers.
			bool isMouseAllowed() const;
			void setMouseAllowed(bool mouseAllowed);
			///If iunset, this button will not generate any wheel triggers.
			bool isWheelAllowed() const;
			void setWheelAllowed(bool wheelAllowed);
			bool isNoButtonAllowed() const;
			void setNoButtonAllowed(bool noButtonAllowed);
			bool showClearButton() const;
			void setShowClearButton(bool showClearButton);
			///If set, a call to setTrigger() will not immediately change the trigger. Instead, the triggerRequest() signal will be fired, and the new trigger will be set only after confirmTrigger() has been called.
			bool requiresValidation() const;
			void setRequiresValidation(bool requiresValidation);
			Palapeli::Trigger trigger() const;
		Q_SIGNALS:
			void triggerChanged(const Palapeli::Trigger& newTrigger);
			void triggerRequest(const Palapeli::Trigger& newTrigger);
		public Q_SLOTS:
			void captureTrigger();
			void clearTrigger();
			void confirmTrigger(const Palapeli::Trigger& trigger);
			void setTrigger(const Palapeli::Trigger& trigger);
		protected:
			bool event(QEvent* event) Q_DECL_OVERRIDE;
		private:
			void updateAppearance();
			void applyTrigger(const Palapeli::Trigger& newTrigger);
			void showModifiers(Qt::KeyboardModifiers modifiers);

			QLabel* m_iconLabel;
			QLabel* m_mainLabel;
			QLabel* m_clearButton;

			Palapeli::Trigger m_trigger, m_stagedTrigger; ///< m_stagedTrigger is the trigger which has been set with setTrigger(), but which is still waiting to be confirmed with confirmTrigger().
			bool m_mouseAllowed, m_wheelAllowed;
			bool m_noButtonAllowed, m_requiresValidation;
	};
}

#endif // PALAPELI_MOUSEINPUTBUTTON_H

/*
    SPDX-FileCopyrightText: 2009 Chani Armitage <chani@kde.org>
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PALAPELI_MOUSEINPUTBUTTON_H
#define PALAPELI_MOUSEINPUTBUTTON_H

#include "../engine/trigger.h"

class QLabel;
#include <QPushButton>

namespace Palapeli
{
	class FlatButton;

	class MouseInputButton : public QPushButton
	{
		Q_OBJECT
		public:
			explicit MouseInputButton(QWidget* parent = nullptr);
			QSize sizeHint() const override;

			///If unset, this button will not generate any wheel triggers.
			bool isMouseAllowed() const;
			void setMouseAllowed(bool mouseAllowed);
			///If iunset, this button will not generate any wheel triggers.
			bool isWheelAllowed() const;
			void setWheelAllowed(bool wheelAllowed);
			bool isNoButtonAllowed() const;
			void setNoButtonAllowed(bool noButtonAllowed);
			bool showClearButton() const;
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
			bool event(QEvent* event) override;
		private:
			void updateAppearance();
			void applyTrigger(const Palapeli::Trigger& newTrigger);
			void showModifiers(Qt::KeyboardModifiers modifiers);

			QLabel* m_iconLabel;
			QLabel* m_mainLabel;
			FlatButton* m_clearButton;

			Palapeli::Trigger m_trigger, m_stagedTrigger; ///< m_stagedTrigger is the trigger which has been set with setTrigger(), but which is still waiting to be confirmed with confirmTrigger().
			bool m_mouseAllowed, m_wheelAllowed;
			bool m_noButtonAllowed, m_requiresValidation;
	};
}

#endif // PALAPELI_MOUSEINPUTBUTTON_H

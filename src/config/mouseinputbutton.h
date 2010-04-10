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

#include "../engine/interactorutils.h"

class QLabel;
#include <QPushButton>
class QToolButton;

namespace Palapeli
{
	class MouseInputButton : public QPushButton
	{
		Q_OBJECT
		public:
			MouseInputButton(QWidget* parent = 0);
			virtual QSize sizeHint() const;

			bool isNoButtonAllowed() const;
			void setNoButtonAllowed(bool noButtonAllowed);
			bool showClearButton() const;
			void setShowClearButton(bool showClearButton);
			///If set, a call to setTrigger() will not immediately change the trigger. Instead, the triggerRequest() signal will be fired, and the new trigger will be set only after confirmTrigger() has been called.
			bool requiresValidation() const;
			void setRequiresValidation(bool requiresValidation);
			Palapeli::InteractorTrigger trigger() const;
		Q_SIGNALS:
			void triggerChanged(const Palapeli::InteractorTrigger& newTrigger);
			void triggerRequest(const Palapeli::InteractorTrigger& newTrigger);
		public Q_SLOTS:
			void captureTrigger();
			void clearTrigger();
			void confirmTrigger(const Palapeli::InteractorTrigger& trigger);
			void setTrigger(const Palapeli::InteractorTrigger& trigger);
		protected:
			bool event(QEvent* event);
		private:
			void updateAppearance();
			void applyTrigger(const Palapeli::InteractorTrigger& newTrigger);
			void showModifiers(Qt::KeyboardModifiers modifiers);

			static const QString DefaultToolTip;

			QLabel* m_iconLabel;
			QLabel* m_mainLabel;
			QLabel* m_clearButton;

			Palapeli::InteractorTrigger m_trigger, m_stagedTrigger; ///< m_stagedTrigger is the trigger which has been set with setTrigger(), but which is still waiting to be confirmed with confirmTrigger().
			bool m_noButtonAllowed, m_requiresValidation;
	};
}

#endif // PALAPELI_MOUSEINPUTBUTTON_H

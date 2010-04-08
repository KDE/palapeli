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

#include <QPushButton>

namespace Palapeli
{
	class MouseInputButton : public QPushButton
	{
		Q_OBJECT
		public:
			MouseInputButton(QWidget* parent = 0);

			void setDefaultText(const QString& text, const QString& toolTip);
			bool isNoButtonAllowed() const;
			void setNoButtonAllowed(bool noButtonAllowed);
			Palapeli::InteractorTrigger trigger() const;
			void setTrigger(const Palapeli::InteractorTrigger& trigger);
		Q_SIGNALS:
			void triggerChanged(const Palapeli::InteractorTrigger& oldTrigger, const Palapeli::InteractorTrigger& newTrigger);
		public Q_SLOTS:
			void reset();
		protected:
			bool event(QEvent* event);
		private Q_SLOTS:
			void getTrigger();
		private:
			void showModifiers(Qt::KeyboardModifiers modifiers);

			Palapeli::InteractorTrigger m_trigger;
			bool m_noButtonAllowed;
			QString m_defaultText;
			QString m_defaultToolTip;
	};
}

#endif // PALAPELI_MOUSEINPUTBUTTON_H

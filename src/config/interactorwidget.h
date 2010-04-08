/***************************************************************************
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

#ifndef PALAPELI_INTERACTORWIDGET_H
#define PALAPELI_INTERACTORWIDGET_H

#include "../engine/interactorutils.h"

class QComboBox;
class QGridLayout;
class QToolButton;

namespace Palapeli
{
	class InteractorManager;
	class MouseInputButton;

	class InteractorWidget : public QObject
	{
		Q_OBJECT
		public:
			//HACK: The parent is a layout because one cannot insert rows into a QGridLayout.
			///\param index the index of the trigger that this widget should represent. If smaller than zero, an empty trigger will be inserted into the InteractorManager, and this 
			InteractorWidget(Palapeli::InteractorManager* manager, int index, QGridLayout* parent);
			virtual ~InteractorWidget();
		private Q_SLOTS:
			void triggerChanged(const Palapeli::InteractorTrigger& trigger);
			void interactorChanged(int interactorIndex);
			void removeRequest();
			void triggerRemovedFromManager(int triggerIndex);
		private:
			//run-time context
			Palapeli::InteractorManager* m_manager;
			int m_index;
			QGridLayout* m_layout;
			//widgets
			Palapeli::MouseInputButton* m_inputButton;
			QComboBox* m_pluginList;
			QToolButton* m_removeButton;
	};
}

#endif // PALAPELI_INTERACTORWIDGET_H

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

#ifndef PALAPELI_INTERACTORMANAGERWIDGET_H
#define PALAPELI_INTERACTORMANAGERWIDGET_H

#include "../engine/interactorutils.h"

class QGridLayout;
class QPushButton;
#include <QWidget>

//TODO: distinguish mouse and wheel interactors

namespace Palapeli
{
	class InteractorManager;
	class InteractorWidget;

	class InteractorManagerWidget : public QWidget
	{
		Q_OBJECT
		public:
			InteractorManagerWidget(Palapeli::InteractorManager* manager, QWidget* parent = 0);
			~InteractorManagerWidget();
		private Q_SLOTS:
			void addRequest();
			void triggerAdded(int index);
			void interactorWidgetDestroyed(QObject* object);
			//TODO: void triggerChanged(int triggerIndex); -> write configuration!
		private:
			void addInteractorWidgetInternal(Palapeli::InteractorWidget* iWidget);

			Palapeli::InteractorManager* m_manager;
			//widgets
			QGridLayout* m_contentsLayout;
			QPushButton* m_addButton;
			QList<Palapeli::InteractorWidget*> m_interactorWidgets;
	};
}

#endif // PALAPELI_INTERACTORMANAGERWIDGET_H

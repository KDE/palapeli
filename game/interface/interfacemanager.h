/***************************************************************************
 *   Copyright (C) 2008 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef PALAPELI_INTERFACEMANAGER_H
#define PALAPELI_INTERFACEMANAGER_H

#include <QObject>
#include <QVariant>

namespace Palapeli
{

	class AutoscalingItem;
	class OnScreenWidget;

	class InterfaceManager : public QObject
	{
		Q_OBJECT
		public:
			enum WidgetType
			{
				NoWidget,
				LoadWidget,
				SaveWidget,
				ExportWidget,
				DeleteWidget
			};

			static InterfaceManager* self();
			void shutdown();
		public Q_SLOTS:
			OnScreenWidget* show(WidgetType type, const QVariantList& args = QVariantList());
			void hide(WidgetType type);
		private Q_SLOTS:
			void next();
		private:
			InterfaceManager();
			~InterfaceManager();
			Q_DISABLE_COPY(InterfaceManager)

			AutoscalingItem* m_autoscaler;
			OnScreenWidget* m_currentWidget;
			WidgetType m_currentWidgetType;
			OnScreenWidget* m_nextWidget;
			WidgetType m_nextWidgetType;
	};

}

//abbreviation for Palapeli::InterfaceManager::self()
inline Palapeli::InterfaceManager* ppIMgr() { return Palapeli::InterfaceManager::self(); }

#endif // PALAPELI_INTERFACEMANAGER_H

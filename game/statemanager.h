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

#ifndef PALAPELI_STATEMANAGER_H
#define PALAPELI_STATEMANAGER_H

#include <QObject>
class KConfig;
class KConfigGroup;

namespace Palapeli
{

	class StateManager : public QObject
	{
		Q_OBJECT
		public:
			StateManager();
			~StateManager();

			QString gameName() const;
			int id() const; //equals the process ID
			bool isPersistent() const;
			
			void setPersistent(bool isPersistent);
		public Q_SLOTS:
			void setGameName(const QString& name);
		private:
			QString m_gameName;
			bool m_isPersistent;
			int m_id;
			KConfig* m_config;
			KConfigGroup* m_configGroup;
	};

}

#endif // PALAPELI_STATEMANAGER_H

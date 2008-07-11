/***************************************************************************
 *   Copyright (C) 2008 Felix Lemke <lemke.felix@ages-skripte.org>
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

#ifndef PALAPELI_SAVEACTION_H
#define PALAPELI_SAVEACTION_H

#include <KActionMenu>
class KMenu;
class KLineEdit;
class KPushButton;

namespace Palapeli
{

	class SaveAction : public KActionMenu
	{
		Q_OBJECT
		public:
			SaveAction(QObject* parent = 0);
			~SaveAction();
		private Q_SLOTS:
			void setPredefinedName(const QString& name);
			void save();
		private:
			KMenu* m_menu;
			KLineEdit* m_nameInput;
			KPushButton* m_saveButton;
			KAction* m_nameInputAct;
			KAction* m_saveAct;
	};

}

#endif // PALAPELI_SAVEACTION_H

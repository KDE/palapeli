/***************************************************************************
 *   Copyright (C) 2009 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef PALAPELI_TEXTUREACTION_H
#define PALAPELI_TEXTUREACTION_H

#include "../macros.h"

#include <KAction>

namespace Palapeli
{

	class ViewMenu;

	class PALAPELIBASE_EXPORT TextureAction : public KAction
	{
		Q_OBJECT
		public:
			TextureAction(Palapeli::ViewMenu* menu, QObject* parent = 0);
		public Q_SLOTS:
			void handleTrigger();
			void handleHidden();
		private:
			Palapeli::ViewMenu* m_menu;
			bool m_showingMenu;
 	};

}

#endif // PALAPELI_TEXTUREACTION_H

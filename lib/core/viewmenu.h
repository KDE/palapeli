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

#ifndef PALAPELI_VIEWMENU_H
#define PALAPELI_VIEWMENU_H

#include "../macros.h"

class QBrush;
#include <KMenu>

namespace Palapeli
{

	class View;
	class ViewMenuPrivate;

	class PALAPELIBASE_EXPORT ViewMenu : public KMenu
	{
		Q_OBJECT
		public:
			ViewMenu(Palapeli::View* view);
			virtual ~ViewMenu();

			QBrush currentBackground();
		public Q_SLOTS:
			void showAtCursorPosition();
		Q_SIGNALS:
			void backgroundSelected(const QBrush& brush);
		private Q_SLOTS:
			void backgroundSelected();
		private:
			Palapeli::ViewMenuPrivate* const p;
	};

}

#endif // PALAPELI_VIEWMENU_H

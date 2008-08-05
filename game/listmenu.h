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

#ifndef PALAPELI_LISTMENU_H
#define PALAPELI_LISTMENU_H

class QAbstractListModel;
#include <QObject>
class QString;
#include <KActionMenu>
class KIcon;

namespace Palapeli
{

	class ListMenuPrivate;

	class ListMenu : public KActionMenu
	{
		Q_OBJECT
		public:
			explicit ListMenu(const KIcon& icon, const QString& text, QObject* parent);
			explicit ListMenu(const QString& text, QObject* parent);
			explicit ListMenu(QObject* parent);
			~ListMenu();

			bool isDisabledWhenEmpty() const;
			bool listMenuEnabled() const;
			QAbstractListModel* model() const;

			void setDisabledWhenEmpty(bool disabledWhenEmpty);
			void setEnabled(bool enabled);
			void setModel(QAbstractListModel* model);
		Q_SIGNALS:
			void clicked(const QString& displayRoleData);
		private:
			ListMenuPrivate* const p;
	};

}

#endif // PALAPELI_LISTMENU_H

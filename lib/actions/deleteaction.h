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

#ifndef PALAPELI_DELETEACTION_H
#define PALAPELI_DELETEACTION_H

#include "../macros.h"

#include <KAction>
#include <KDialog>

namespace Palapeli
{

	class Library;
	class LibraryView;

	class PALAPELIBASE_EXPORT DeleteDialog : public KDialog
	{
		Q_OBJECT
		public:
			DeleteDialog(Palapeli::Library* mainLibrary);
			~DeleteDialog();
		public Q_SLOTS:
			void handleOkButton();
			void checkItemVisibility();
		Q_SIGNALS:
			void hintActionState(bool enabled);
		protected:
			virtual void showEvent(QShowEvent* event);
		private:
			Palapeli::LibraryView* m_mainLibraryView;
	};

	class PALAPELIBASE_EXPORT DeleteAction : public KAction
	{
		Q_OBJECT
		public:
			DeleteAction(QObject* parent);
			~DeleteAction();
		public Q_SLOTS:
			void createDialog();
		private:
			Palapeli::DeleteDialog* m_dialog;
	};

}

#endif // PALAPELI_DELETEACTION_H

/***************************************************************************
 *   Copyright 2008 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef PALAPELI_EXPORTACTION_H
#define PALAPELI_EXPORTACTION_H

#include "../macros.h"

#include <KAction>
#include <KDialog>

namespace Palapeli
{

	class Library;
	class LibraryView;

	class PALAPELIBASE_EXPORT ExportDialog : public KDialog
	{
		Q_OBJECT
		public:
			ExportDialog(Palapeli::Library* mainLibrary);
			~ExportDialog();
		public Q_SLOTS:
			void handleOkButton();
		protected:
			virtual void showEvent(QShowEvent* event);
		private:
			Palapeli::LibraryView* m_mainLibraryView;
	};

	class PALAPELIBASE_EXPORT ExportAction : public KAction
	{
		Q_OBJECT
		public:
			ExportAction(QObject* parent);
			~ExportAction();
		public Q_SLOTS:
			void handleTrigger();
		private:
			Palapeli::ExportDialog* m_dialog;
	};

}

#endif // PALAPELI_EXPORTACTION_H

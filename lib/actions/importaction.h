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

#ifndef PALAPELI_IMPORTACTION_H
#define PALAPELI_IMPORTACTION_H

#include "../macros.h"

#include <KAction>
#include <KDialog>

namespace Palapeli
{

	class Library;
	class LibraryArchiveBase;
	class LibraryView;

	class PALAPELIBASE_EXPORT ImportDialog : public KDialog
	{
		Q_OBJECT
		public:
			ImportDialog(const KUrl& url);
			~ImportDialog();
			bool isArchiveValid() const;
		public Q_SLOTS:
			void handleOkButton();
		private:
			LibraryArchiveBase* m_archiveBase;
			Library* m_archiveLibrary;
			LibraryView* m_archiveLibraryView;
	};

	class PALAPELIBASE_EXPORT ImportAction : public KAction
	{
		Q_OBJECT
		public:
			ImportAction(QObject* parent);
		public Q_SLOTS:
			void handleTrigger();
	};

}

#endif // PALAPELI_IMPORTACTION_H

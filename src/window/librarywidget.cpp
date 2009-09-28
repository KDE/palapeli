/***************************************************************************
 *   Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
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

#include "librarywidget.h"
#include "../file-io/libraryview.h"

#include <KAction>
#include <KActionCollection>
#include <KLocalizedString>

Palapeli::LibraryWidget::LibraryWidget()
	: Palapeli::TabWindow(QLatin1String("palapeli-library"))
	, m_view(new Palapeli::LibraryView)
{
	//setup actions
	KAction* importAct = new KAction(KIcon("document-import"), i18n("&Import..."), 0);
	importAct->setEnabled(false); //not implemented yet
	importAct->setToolTip(i18n("Import a new puzzle from a file"));
	actionCollection()->addAction("file_import", importAct);
	//setup GUI
	setupGUI();
	setCentralWidget(m_view);
}

Palapeli::LibraryView* Palapeli::LibraryWidget::view() const
{
	return m_view;
}

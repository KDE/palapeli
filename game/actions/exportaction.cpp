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

#include "exportaction.h"
#include "../library/library.h"
#include "../library/librarybase.h"
#include "../library/libraryview.h"
#include "../library/puzzleinfo.h"
#include "../manager.h"
#include "../mainwindow.h"

#include <KActionCollection>
#include <KFileDialog>
#include <KIcon>
#include <KLocalizedString>

//BEGIN Palapeli::ExportDialog

Palapeli::ExportDialog::ExportDialog(Palapeli::Library* mainLibrary)
	: m_mainLibraryView(new Palapeli::LibraryView(mainLibrary))
{
	setCaption(i18n("Export a puzzle from your library - Palapeli"));
	setButtons(KDialog::Ok | KDialog::Cancel);
	setMainWidget(m_mainLibraryView);
	resize(600, 400);
	connect(this, SIGNAL(okClicked()), this, SLOT(handleOkButton()));
	connect(m_mainLibraryView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(handleOkButton()));
}

Palapeli::ExportDialog::~ExportDialog()
{
	delete m_mainLibraryView;
}

void Palapeli::ExportDialog::handleOkButton()
{
	hide();
	//get puzzle identifier
	Palapeli::PuzzleInfo* info = m_mainLibraryView->puzzleInfo();
	if (!info)
		return;
	if (info->identifier.isEmpty())
		return;
	//get target URL
	KUrl target = KFileDialog::getSaveUrl(KUrl("kfiledialog:///palapeli"), "*.pala|" + i18nc("Used as filter description in a file dialog.", "Palapeli Puzzle (*.pala)"), ppMgr()->window(), i18nc("Used as caption for file dialog.", "Select file to export selected puzzle to - Palapeli"));
	if (target.isEmpty()) //process aborted by user
		return;
	//export to given target
	Palapeli::LibraryArchiveBase archive(target);
	archive.create(m_mainLibraryView->library(), info->identifier);
}

//END Palapeli::ExportDialog

//BEGIN Palapeli::ExportAction

Palapeli::ExportAction::ExportAction(QObject* parent)
	: KAction(KIcon("document-export"), i18n("&Export"), parent)
	, m_dialog(new Palapeli::ExportDialog(ppMgr()->library()))
{
	setObjectName("palapeli_export");
	setShortcut(KShortcut(Qt::CTRL + Qt::Key_E));
	setToolTip(i18n("Export a puzzle from your library to an archive file"));
	connect(this, SIGNAL(triggered()), m_dialog, SLOT(show()));

	KActionCollection* collection = qobject_cast<KActionCollection*>(parent);
	if (collection)
		collection->addAction(objectName(), this);
}

Palapeli::ExportAction::~ExportAction()
{
	delete m_dialog;
}

//END Palapeli::ExportAction

#include "exportaction.moc"

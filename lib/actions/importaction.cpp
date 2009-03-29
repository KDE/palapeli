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

#include "importaction.h"
#include "commonaction.h"
#include "../library/library.h"
#include "../library/librarybase.h"
#include "../library/libraryview.h"

#include <KActionCollection>
#include <KFileDialog>
#include <KLocalizedString>
#include <KMessageBox>

//BEGIN Palapeli::ImportDialog

Palapeli::ImportDialog::ImportDialog(const KUrl& url)
	: KDialog(Palapeli::Actions::dialogParent())
	, m_archiveBase(new Palapeli::LibraryArchiveBase(url))
	, m_archiveLibrary(0) //will be initialized later
	, m_archiveLibraryView(0)
{
	if (!isArchiveValid())
	{
		KMessageBox::error(Palapeli::Actions::dialogParent(), i18n("This puzzle archive is unreadable. It might be corrupted."));
		return;
	}
	m_archiveLibrary = new Palapeli::Library(m_archiveBase);
	m_archiveLibraryView = new Palapeli::LibraryView(m_archiveLibrary);
	m_archiveLibraryView->setSelectionMode(QAbstractItemView::NoSelection);

	setCaption(i18n("Import this puzzle into your library?"));
	setButtons(KDialog::Ok | KDialog::Cancel);
	setMainWidget(m_archiveLibraryView);
	resize(600, 150);
	connect(this, SIGNAL(okClicked()), this, SLOT(handleOkButton()));
}

Palapeli::ImportDialog::~ImportDialog()
{
	delete m_archiveLibraryView;
	delete m_archiveLibrary;
	delete m_archiveBase;
}

bool Palapeli::ImportDialog::isArchiveValid() const
{
	return m_archiveBase->findEntries().count() == 1;
}

void Palapeli::ImportDialog::handleOkButton()
{
	Palapeli::LibraryStandardBase::self()->insertEntries(m_archiveLibrary);
}

//END Palapeli::ImportDialog

//BEGIN Palapeli::ImportAction

Palapeli::ImportAction::ImportAction(QObject* parent)
	: KAction(KIcon("document-import"), i18n("&Import"), parent)
{
	setObjectName("palapeli_import");
	setShortcut(KShortcut(Qt::CTRL + Qt::Key_I));
	setToolTip(i18n("Import a puzzle from an archive file into your library"));
	connect(this, SIGNAL(triggered(bool)), this, SLOT(handleTrigger()));

	KActionCollection* collection = qobject_cast<KActionCollection*>(parent);
	if (collection)
		collection->addAction(objectName(), this);
}

void Palapeli::ImportAction::handleTrigger()
{
	//get source URL
	const KUrl source = KFileDialog::getOpenUrl(KUrl("kfiledialog:///palapeli"), "*.pala|" + i18nc("Used as filter description in a file dialog.", "Palapeli Puzzle (*.pala)"), Palapeli::Actions::dialogParent(), i18nc("Used as caption for file dialog.", "Select puzzle archive - Palapeli"));
	if (source.isEmpty()) //process aborted by user
		return;
	//create a dialog which confirms the action
	Palapeli::ImportDialog dialog(source);
	if (dialog.isArchiveValid())
		dialog.exec();
}

//END Palapeli::ImportAction

#include "importaction.moc"

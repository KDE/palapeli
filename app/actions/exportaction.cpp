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
#include "commonaction.h"
#include "../../lib/library/library.h"
#include "../../lib/library/librarybase.h"
#include "../../lib/library/libraryview.h"
#include "../../lib/library/puzzleinfo.h"

#include <KActionCollection>
#include <KFileDialog>
#include <KIcon>
#include <KLocalizedString>

//BEGIN Palapeli::ExportDialog

Palapeli::ExportDialog::ExportDialog(Palapeli::Library* mainLibrary)
	: KDialog(Palapeli::Actions::dialogParent())
	, m_mainLibraryView(new Palapeli::LibraryView(mainLibrary))
{
	setCaption(i18n("Export a puzzle from your library"));
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
	const KUrl target = KFileDialog::getSaveUrl(KUrl("kfiledialog:///palapeli"), "*.pala|" + i18nc("Used as filter description in a file dialog.", "Palapeli Puzzle (*.pala)"), Palapeli::Actions::dialogParent(), i18nc("Used as caption for file dialog.", "Choose file to export selected puzzle to - Palapeli"));
	if (target.isEmpty()) //process aborted by user
		return;
	//export to given target
	Palapeli::LibraryArchiveBase archive(target);
	archive.insertEntry(info->identifier, m_mainLibraryView->library());
}

void Palapeli::ExportDialog::showEvent(QShowEvent* event)
{
	Q_UNUSED(event)
	m_mainLibraryView->setFocus(Qt::OtherFocusReason);
	QModelIndex standardSelection = m_mainLibraryView->model()->index(0, 0);
	m_mainLibraryView->selectionModel()->select(standardSelection, QItemSelectionModel::ClearAndSelect);
	m_mainLibraryView->scrollTo(standardSelection);
}

//END Palapeli::ExportDialog

//BEGIN Palapeli::ExportAction

Palapeli::ExportAction::ExportAction(QObject* parent)
	: KAction(KIcon("document-export"), i18n("&Export"), parent)
	, m_dialog(0)
{
	setObjectName("palapeli_export");
	setShortcut(KShortcut(Qt::CTRL + Qt::Key_E));
	setToolTip(i18n("Export a puzzle from your library to an archive file"));
	connect(this, SIGNAL(triggered()), this, SLOT(handleTrigger()));

	KActionCollection* collection = qobject_cast<KActionCollection*>(parent);
	if (collection)
		collection->addAction(objectName(), this);
}

Palapeli::ExportAction::~ExportAction()
{
	delete m_dialog;
}

void Palapeli::ExportAction::handleTrigger()
{
	if (!m_dialog)
		m_dialog = new Palapeli::ExportDialog(Palapeli::standardLibrary());
	m_dialog->show();
}

//END Palapeli::ExportAction

#include "exportaction.moc"

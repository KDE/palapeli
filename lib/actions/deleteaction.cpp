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

#include "deleteaction.h"
#include "commonaction.h"
#include "../library/library.h"
#include "../library/librarybase.h"
#include "../library/libraryview.h"
#include "../library/puzzleinfo.h"

#include <QTimer>
#include <KActionCollection>
#include <KFileDialog>
#include <KIcon>
#include <KLocalizedString>
#include <KMessageBox>

//BEGIN Palapeli::DeleteDialog

Palapeli::DeleteDialog::DeleteDialog(Palapeli::Library* mainLibrary)
	: KDialog(Palapeli::Actions::dialogParent())
	, m_mainLibraryView(new Palapeli::LibraryView(mainLibrary))
{
	setCaption(i18n("Delete a puzzle from your library"));
	setButtons(KDialog::Ok | KDialog::Cancel);
	setMainWidget(m_mainLibraryView);
	m_mainLibraryView->setDeletionFilterEnabled(true);
	resize(600, 400);
	connect(this, SIGNAL(okClicked()), this, SLOT(handleOkButton()));
	connect(m_mainLibraryView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(handleOkButton()));
	connect(mainLibrary, SIGNAL(rowsInserted(const QModelIndex&, int, int)), this, SLOT(checkItemVisibility()));
	connect(mainLibrary, SIGNAL(rowsRemoved(const QModelIndex&, int, int)), this, SLOT(checkItemVisibility()));
}

Palapeli::DeleteDialog::~DeleteDialog()
{
	delete m_mainLibraryView;
}

void Palapeli::DeleteDialog::handleOkButton()
{
	hide();
	//get puzzle identifier
	Palapeli::PuzzleInfo* info = m_mainLibraryView->puzzleInfo();
	if (!info)
		return;
	if (info->identifier.isEmpty())
		return;
	//confirm deletion
	int result = KMessageBox::warningContinueCancel(Palapeli::Actions::dialogParent(), i18n("You're about to delete puzzle \"%1\" from your library.", info->name), i18n("Confirm deletion"), KStandardGuiItem::cont(), KStandardGuiItem::cancel(), QLatin1String("confirm-library-deletion"));
	//perform deletion
	if (result == KMessageBox::Continue)
		m_mainLibraryView->library()->base()->removeEntry(info->identifier, m_mainLibraryView->library());
}

void Palapeli::DeleteDialog::checkItemVisibility()
{
	emit hintActionState(m_mainLibraryView->model()->rowCount(QModelIndex()) > 0);
}

void Palapeli::DeleteDialog::showEvent(QShowEvent* event)
{
	Q_UNUSED(event)
	m_mainLibraryView->setFocus(Qt::OtherFocusReason);
	QModelIndex standardSelection = m_mainLibraryView->model()->index(0, 0);
	m_mainLibraryView->selectionModel()->select(standardSelection, QItemSelectionModel::ClearAndSelect);
	m_mainLibraryView->scrollTo(standardSelection);
}

//END Palapeli::DeleteDialog

//BEGIN Palapeli::DeleteAction

Palapeli::DeleteAction::DeleteAction(QObject* parent)
	: KAction(KIcon("edit-delete-page"), i18n("&Delete"), parent)
	, m_dialog(0)
{
	setObjectName("palapeli_delete");
	setShortcut(KShortcut(Qt::CTRL + Qt::Key_D));
	setToolTip(i18n("Delete a puzzle from your library"));

	QTimer::singleShot(0, this, SLOT(createDialog()));

	KActionCollection* collection = qobject_cast<KActionCollection*>(parent);
	if (collection)
		collection->addAction(objectName(), this);
}

Palapeli::DeleteAction::~DeleteAction()
{
	delete m_dialog;
}

void Palapeli::DeleteAction::createDialog()
{
	m_dialog = new Palapeli::DeleteDialog(Palapeli::standardLibrary());
	connect(this, SIGNAL(triggered()), m_dialog, SLOT(show()));
	connect(m_dialog, SIGNAL(hintActionState(bool)), this, SLOT(setEnabled(bool)));
	m_dialog->checkItemVisibility();
}

//END Palapeli::DeleteAction

#include "deleteaction.moc"

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

#include "loadaction.h"
#include "../library/library.h"
#include "../library/libraryview.h"
#include "../manager.h"

#include <KActionCollection>
#include <KIcon>
#include <KLocalizedString>
#include <KStandardShortcut>

//BEGIN Palapeli::LoadDialog

Palapeli::LoadDialog::LoadDialog(Palapeli::Library* mainLibrary)
	: m_mainLibraryView(new Palapeli::LibraryView(mainLibrary))
{
	setCaption(i18n("Load a puzzle from the library"));
	setButtons(KDialog::Ok | KDialog::Cancel);
	setMainWidget(m_mainLibraryView);
	resize(600, 400);
	connect(this, SIGNAL(okClicked()), this, SLOT(handleOkButton()));
	connect(m_mainLibraryView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(handleOkButton()));
}

Palapeli::LoadDialog::~LoadDialog()
{
	delete m_mainLibraryView;
}

void Palapeli::LoadDialog::handleOkButton()
{
	Palapeli::PuzzleInfo* info = m_mainLibraryView->puzzleInfo();
	if (info)
		ppMgr()->loadGame(info);
	hide();
}

void Palapeli::LoadDialog::showEvent(QShowEvent* event)
{
	Q_UNUSED(event)
	m_mainLibraryView->setFocus(Qt::OtherFocusReason);
	QModelIndex standardSelection = m_mainLibraryView->model()->index(0, 0);
	m_mainLibraryView->selectionModel()->select(standardSelection, QItemSelectionModel::ClearAndSelect);
	m_mainLibraryView->scrollTo(standardSelection);
}

//END Palapeli::LoadDialog

//BEGIN Palapeli::LoadAction

Palapeli::LoadAction::LoadAction(QObject* parent)
	: KAction(KIcon("document-load"), i18n("&Open"), parent)
	, m_dialog(0)
{
	setObjectName("palapeli_load");
	setShortcut(KStandardShortcut::shortcut(KStandardShortcut::Open));
	setToolTip(i18n("Open a puzzle from your library"));
	connect(this, SIGNAL(triggered()), this, SLOT(handleTrigger()));

	KActionCollection* collection = qobject_cast<KActionCollection*>(parent);
	if (collection)
		collection->addAction(objectName(), this);
}

Palapeli::LoadAction::~LoadAction()
{
	delete m_dialog;
}

void Palapeli::LoadAction::handleTrigger()
{
	if (!m_dialog)
		m_dialog = new Palapeli::LoadDialog(ppMgr()->library());
	m_dialog->show();
}

//END Palapeli::LoadAction

#include "loadaction.moc"

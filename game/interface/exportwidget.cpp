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

#include "exportwidget.h"
#include "interfacemanager.h"
#include "../mainwindow.h"
#include "../manager.h"
#include "../savegamemodel.h"
#include "../../storage/gamestorageattribs.h"
#include "../../storage/gamestorage.h"
#include "../../storage/gamestorageitem.h"

#include <QListView>
#include <KActionCollection>
#include <KFileDialog>
#include <KLocalizedString>
#include <KShortcut>
#include <KStandardGuiItem>

//BEGIN Palapeli::ExportWidget

Palapeli::ExportWidget* Palapeli::ExportWidget::create(Palapeli::AutoscalingItem* parent)
{
	QListView* view = new QListView;
	view->setModel(ppMgr()->savegameModel());
	view->setSelectionMode(QAbstractItemView::MultiSelection);
	return new Palapeli::ExportWidget(view, parent);
}

Palapeli::ExportWidget::ExportWidget(QListView* view, Palapeli::AutoscalingItem* parent)
	: Palapeli::OnScreenDialog(view, QList<KGuiItem>() << KStandardGuiItem::save() << KStandardGuiItem::cancel(), i18n("Select saved games to export"), parent)
	, m_view(view)
{
	connect(this, SIGNAL(buttonPressed(int)), this, SLOT(handleButton(int)));
	connect(m_view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(handleSelectionChange()));
	setButtonEnabled(0, false); //nothing selected to export by now
	update();
}

void Palapeli::ExportWidget::handleButton(int id)
{
	if (id == 0) //the "Save" button
	{
		Palapeli::GameStorage gs;
		//gather a list of all game items to export
		Palapeli::GameStorageItems exportableItems;
		foreach (const QModelIndex& index, m_view->selectionModel()->selectedIndexes())
		{
			QString gameName = ppMgr()->savegameModel()->data(index, Qt::DisplayRole).toString();
			exportableItems += gs.queryItems(Palapeli::GameStorageAttributes() << new Palapeli::GameStorageMetaAttribute(gameName) << new Palapeli::GameStorageTypeAttribute(Palapeli::GameStorageItem::SavedGame));
		}
		if (exportableItems.count() == 0)
			return;
		//ask the user for a file name
		KUrl target = KFileDialog::getSaveUrl(KUrl("kfiledialog:///palapeli"), "*.psga|" + i18nc("Used as filter description in a file dialog.", "Palapeli Saved Game Archive (*.psga)"), ppMgr()->window(), i18nc("Used as caption for file dialog.", "Select file to export selected games to - Palapeli"));
		if (target.isEmpty()) //process aborted by user
			return;
		gs.exportItems(target, exportableItems);
	}
	ppIMgr()->hide(Palapeli::InterfaceManager::ExportWidget);
}

void Palapeli::ExportWidget::handleSelectionChange()
{
	setButtonEnabled(0, m_view->selectionModel()->hasSelection());
}

//END Palapeli::ExportWidget

//BEGIN Palapeli::ExportWidgetAction

Palapeli::ExportWidgetAction::ExportWidgetAction(QObject* parent)
	: KAction(KIcon("document-export"), i18n("&Export"), parent)
{
	setObjectName("palapeli_export");
	setShortcut(KShortcut(Qt::CTRL + Qt::Key_E));
	setToolTip(i18n("Export saved games to an archive file"));

	connect(this, SIGNAL(triggered(bool)), this, SLOT(trigger()));
	connect(ppMgr(), SIGNAL(savegameCreated(const QString&)), this, SLOT(gameCountChanged()));
	connect(ppMgr(), SIGNAL(savegameDeleted(const QString&)), this, SLOT(gameCountChanged()));
	gameCountChanged(); //adapt to initial state

	KActionCollection* collection = qobject_cast<KActionCollection*>(parent);
	if (collection)
		collection->addAction(objectName(), this);
}

void Palapeli::ExportWidgetAction::gameCountChanged()
{
	setEnabled(ppMgr()->savegameModel()->savegameCount() > 0);
}

void Palapeli::ExportWidgetAction::trigger()
{
	ppIMgr()->show(Palapeli::InterfaceManager::ExportWidget);
}

//END Palapeli::ExportWidgetAction

#include "exportwidget.moc"

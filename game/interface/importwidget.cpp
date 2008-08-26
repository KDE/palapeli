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

#include "importwidget.h"
#include "interfacemanager.h"
#include "../mainwindow.h"
#include "../manager.h"
#include "../savegamemodel.h"
#include "../../storage/gamestorageattribs.h"
#include "../../storage/gamestorage.h"
#include "../../storage/gamestorageitem.h"

#include <KActionCollection>
#include <KFileDialog>
#include <KLocalizedString>
#include <KShortcut>
#include <KStandardGuiItem>

//BEGIN Palapeli::ImportWidgetAction

Palapeli::ImportWidgetAction::ImportWidgetAction(QObject* parent)
	: KAction(KIcon("document-import"), i18n("&Import"), parent)
{
	setObjectName("palapeli_import");
	setShortcut(KShortcut(Qt::CTRL + Qt::Key_I));
	setToolTip(i18n("Import saved games from an archive file"));

	connect(this, SIGNAL(triggered(bool)), this, SLOT(trigger()));

	KActionCollection* collection = qobject_cast<KActionCollection*>(parent);
	if (collection)
		collection->addAction(objectName(), this);
}

void Palapeli::ImportWidgetAction::trigger()
{
		//ask the user for a target
	KUrl target = KFileDialog::getOpenUrl(KUrl("kfiledialog:///palapeli"), "*.psga|" + i18nc("Used as filter description in a file dialog.", "Palapeli Saved Game Archive (*.psga)"), ppMgr()->window(), i18nc("Used as caption for file dialog.", "Select archive to import games from - Palapeli"));
	if (target.isEmpty()) //process aborted by user
		return;
	Palapeli::GameStorage gs;
	connect(&gs, SIGNAL(progress(int, int, int, const QString&)), ppMgr()->window(), SLOT(reportProgress(int, int, int, const QString&)));
	Palapeli::GameStorageItems importedItems = gs.importItems(target, true, QLatin1String(".psg"));
	ppMgr()->window()->flushProgress(2);
	foreach (const Palapeli::GameStorageItem& item, importedItems)
	{
		if (item.type() == Palapeli::GameStorageItem::SavedGame)
			ppMgr()->savegameWasCreated(item.metaData());
	}
	//show load widget to let the user select one of the imported games
	if (ppMgr()->savegameModel()->savegameCount() > 0)
		ppIMgr()->show(Palapeli::InterfaceManager::LoadWidget);
}

//END Palapeli::ImportWidgetAction

#include "importwidget.moc"

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

#include "kns-downloadaction.h"
#include "../library/library.h"
#include "../library/librarybase.h"
#include "../library/puzzleinfo.h"

#include <QFile>
#include <KActionCollection>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <knewstuff2/ui/knewstuffaction.h>
#include <knewstuff2/engine.h>
#include <KStandardDirs>

Palapeli::KnsDownloadAction::KnsDownloadAction(QObject* parent)
	: KAction(KIcon(""), i18n("Down&load new puzzles"), parent)
{
	setObjectName("palapeli_knsdownload");
	setToolTip(i18n("Get hot new puzzles"));

	KActionCollection* collection = qobject_cast<KActionCollection*>(parent);
	if (collection)
		collection->addAction(objectName(), this);
}

void Palapeli::KnsDownloadAction::handleTrigger()
{
	KNS::Entry::List entries = KNS::Engine::download();
	if (entries.isEmpty())
		return;
	//apply new entry status
	Palapeli::LibraryArchiveBase* archive; Palapeli::Library* library;
	KConfig config(KStandardDirs::locateLocal("config", "palapeli-ghnsrc"));
	KConfigGroup configGroup(&config, "GHNS Registry");
	QString identifier;
	foreach (KNS::Entry* entry, entries)
	{
		const QString payload = entry->payload().representation();
		switch (entry->status())
		{
			case KNS::Entry::Installed:
				if (entry->installedFiles().isEmpty())
					break;
				//merge downloaded .pala archive into standard library
				archive = new Palapeli::LibraryArchiveBase(KUrl(entry->installedFiles()[0]));
				library = new Palapeli::Library(archive);
				Palapeli::LibraryStandardBase::self()->insertEntries(library);
				for (int i = 0; i < library->rowCount(); ++i)
				{
					//report new entry to standard library
					Palapeli::LibraryStandardBase::self()->reportNewEntry(library->infoForPuzzle(i)->identifier);
					//save association payload<->puzzle identifier (needed later on)
					configGroup.writeEntry(library->infoForPuzzle(i)->identifier, payload);
				}
				delete library;
				delete archive;
				//the downloaded archive is not needed anymore
				QFile(entry->installedFiles()[0]).remove();
				break;
			case KNS::Entry::Deleted:
				//find associated puzzle identifiers
				for (int i = 0; i < Palapeli::standardLibrary()->rowCount(); ++i)
				{
					identifier = Palapeli::standardLibrary()->infoForPuzzle(i)->identifier;
					if (configGroup.readEntry(identifier, QString()) == payload)
					{
						//remove selected puzzle because it is associated with this GHNS entry
						Palapeli::LibraryStandardBase::self()->removeEntry(identifier, Palapeli::standardLibrary());
						//remove entry from GHNS registry
						configGroup.deleteEntry(identifier);
					}
				}
				break;
			default: //other status changes are irrelevant for us
				break;
		}
	}
}

#include "kns-downloadaction.moc"

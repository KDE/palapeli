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

#include "importhelper.h"
#include "file-io/collection-filesystem.h"
#include "file-io/collection-list.h"
#include "file-io/puzzle-old.h"

#include <QApplication>
#include <QTimer>
#include <KCmdLineArgs>
#include <KDebug> //we use kError
#include <KLocalizedString>
#include <KMessageBox>
#include <KNotification>

Palapeli::ImportHelper::ImportHelper(KCmdLineArgs* args)
	: m_args(args)
	, m_fileSystemCollection(new Palapeli::FileSystemCollection)
	, m_localCollection(new Palapeli::LocalCollection)
{
	QTimer::singleShot(0, this, SLOT(doWork()));
}

void Palapeli::ImportHelper::doWork()
{
	if (m_args->count() == 0)
	{
		kError() << i18nc("command line message", "Error: No puzzle file given.");
		qApp->quit();
	}
	//try to load puzzle
	QModelIndex index = m_fileSystemCollection->providePuzzle(m_args->url(0));
	QObject* puzzlePayload = index.data(Palapeli::Collection::PuzzleObjectRole).value<QObject*>();
	Palapeli::OldPuzzle* puzzle = qobject_cast<Palapeli::OldPuzzle*>(puzzlePayload);
	if (!puzzle)
	{
		KMessageBox::sorry(0, i18n("The given puzzle file is corrupted."));
		qApp->quit();
	}
	//do import
	const QModelIndex newIndex = m_localCollection->importPuzzle(puzzle);
	if (!newIndex.isValid())
	{
		KMessageBox::sorry(0, i18n("The puzzle file could not be imported into the local collection."));
		qApp->quit();
	}
	//show notification
	KNotification::event(QLatin1String("importingPuzzle"),
		i18n("Importing puzzle \"%1\" into your collection", puzzle->metadata()->name),
		QPixmap::fromImage(puzzle->metadata()->thumbnail)
	);
	//keep program running until the puzzle has been written
	QObject* newPuzzlePayload = newIndex.data(Palapeli::Collection::PuzzleObjectRole).value<QObject*>();
	connect(newPuzzlePayload, SIGNAL(writeFinished()), qApp, SLOT(quit()));
}

Palapeli::ImportHelper::~ImportHelper()
{
	delete m_fileSystemCollection;
	delete m_localCollection;
}

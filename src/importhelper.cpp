/***************************************************************************
 *   Copyright 2009-2011 Stefan Majewsky <majewsky@gmx.net>
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
#include "file-io/collection.h"
#include "file-io/components.h"
#include "file-io/puzzle.h"

#include <QtCore/QFutureWatcher>
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <KDE/KCmdLineArgs>
#include <KDE/KDebug> //we use kError
#include <KDE/KNotification>

Palapeli::ImportHelper::ImportHelper(KCmdLineArgs* args)
	: m_args(args)
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
	//import puzzle
	Palapeli::Puzzle* puzzle = Palapeli::Collection::instance()->importPuzzle(m_args->arg(0));
	//show notification
	puzzle->get(Palapeli::PuzzleComponent::Metadata).waitForFinished();
	const Palapeli::MetadataComponent* cmp = puzzle->component<Palapeli::MetadataComponent>();
	if (cmp)
	{
		KNotification::event(QLatin1String("importingPuzzle"),
			i18n("Importing puzzle \"%1\" into your collection", cmp->metadata.name),
			QPixmap::fromImage(cmp->metadata.thumbnail)
		);
	}
	//keep program running until the puzzle has been written
	Palapeli::FutureWatcher* watcher = new Palapeli::FutureWatcher;
	connect(watcher, SIGNAL(finished()), watcher, SLOT(deleteLater()));
	connect(watcher, SIGNAL(finished()), qApp, SLOT(quit()));
	watcher->setFuture(puzzle->get(Palapeli::PuzzleComponent::ArchiveStorage));
}

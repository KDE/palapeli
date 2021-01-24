/*
    SPDX-FileCopyrightText: 2009-2011 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "importhelper.h"
#include "file-io/collection.h"
#include "file-io/components.h"
#include "file-io/puzzle.h"

#include <QTimer>
#include <QApplication>
#include "palapeli_debug.h"
#include <KLocalizedString>
#include <KNotification>

Palapeli::ImportHelper::ImportHelper(const QString &path)
    : m_path(path)
{
	QTimer::singleShot(0, this, &ImportHelper::doWork);
}

void Palapeli::ImportHelper::doWork()
{
    if (m_path.isEmpty())
	{
		qCCritical(PALAPELI_LOG) << i18nc("command line message", "Error: No puzzle file given.");
		qApp->quit();
	}
	//import puzzle
    Palapeli::Puzzle* puzzle = Palapeli::Collection::instance()->importPuzzle(m_path);
	//show notification
	puzzle->get(Palapeli::PuzzleComponent::Metadata);
	const Palapeli::MetadataComponent* cmp = puzzle->component<Palapeli::MetadataComponent>();
	if (cmp)
	{
		KNotification::event(QStringLiteral("importingPuzzle"),
			i18n("Importing puzzle \"%1\" into your collection", cmp->metadata.name),
			QPixmap::fromImage(cmp->metadata.thumbnail)
		);
	}
	puzzle->get(Palapeli::PuzzleComponent::ArchiveStorage);
	qApp->quit ();
}

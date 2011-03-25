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

#include "collection-list.h"
#include "components.h"
#include "puzzle.h"
#include "puzzle-old.h"

#include <QFile>
#include <QUuid>
#include <KConfig>
#include <KConfigGroup>
#include <KStandardDirs>

static KUrl readUrl(const KUrl& url)
{
	if (url.protocol() == "palapeli")
	{
		//resolve special URLs with KStandardDirs
		QString path = url.path(KUrl::RemoveTrailingSlash);
		if (path.startsWith('/'))
			path.remove(0, 1);
		return KUrl(KStandardDirs::locate("appdata", path));
	}
	else
		return url;
}

Palapeli::LocalCollection::LocalCollection()
	: m_config(new KConfig("palapeli-collectionrc", KConfig::CascadeConfig))
	, m_group(new KConfigGroup(m_config, "Palapeli Collection"))
{
	//read the puzzles
	const QStringList puzzleIds = m_group->groupList();
	foreach (const QString& puzzleId, puzzleIds)
	{
		KConfigGroup* puzzleGroup = new KConfigGroup(m_group, puzzleId);
		//construct puzzle
		KUrl url = readUrl(puzzleGroup->readEntry("Location", KUrl()));
		if (url.isEmpty())
			continue;
		addPuzzle(new Palapeli::OldPuzzle(new Palapeli::CollectionStorageComponent(puzzleGroup), url, puzzleId));
	}
}

Palapeli::LocalCollection::~LocalCollection()
{
	delete m_config;
	delete m_group;
}

QModelIndex Palapeli::LocalCollection::storeGeneratedPuzzle(Palapeli::OldPuzzle* puzzle)
{
	if (!puzzle)
		return QModelIndex();
	if (!puzzle->location().isEmpty())
		return QModelIndex();
	return importPuzzleInternal(puzzle);
}

bool Palapeli::LocalCollection::canImportPuzzles() const
{
	return true;
}

QModelIndex Palapeli::LocalCollection::importPuzzle(const Palapeli::OldPuzzle* const puzzle)
{
	if (!puzzle->metadata())
		return QModelIndex();
	//create a writable copy of the given puzzle, and import this into the collection
	Palapeli::OldPuzzle* newPuzzle = new Palapeli::OldPuzzle(*puzzle, QUuid::createUuid().toString());
	return importPuzzleInternal(newPuzzle);
}

QModelIndex Palapeli::LocalCollection::importPuzzleInternal(Palapeli::OldPuzzle* puzzle)
{
	//determine location of new puzzle
	const QString identifier = puzzle->newPuzzle()->identifier();
	const QString fileName = QString("collection/%1.puzzle").arg(identifier);
	const KUrl location(KStandardDirs::locateLocal("appdata", fileName));
	//create a copy of the given puzzle, and relocate it to the new location
	puzzle->setLocation(location);
	puzzle->write();
	//create the config group for this puzzle
	KConfigGroup mainGroup(m_config, "Palapeli Collection");
	KConfigGroup puzzleGroup(&mainGroup, identifier);
	puzzleGroup.writeEntry("Location", QString("palapeli:///%1").arg(fileName)); //we use a pseudo-URL here, to avoid problems when the configuration directory is relocated
	m_config->sync();
	//add to the internal storage
	return addPuzzle(puzzle);
}

bool Palapeli::LocalCollection::canDeletePuzzle(const QModelIndex& index) const
{
	//get puzzle object
	QObject* puzzlePayload = index.data(PuzzleObjectRole).value<QObject*>();
	Palapeli::OldPuzzle* puzzle = qobject_cast<Palapeli::OldPuzzle*>(puzzlePayload);
	if (!puzzle || !puzzle->metadata())
		return false;
	//check whether that particular puzzle can be removed
	return !puzzle->metadata()->modifyProtection;
	//NOTE: This is a protection for the default puzzles. In user-generated puzzles, ModifyProtection is not enabled.
}

bool Palapeli::LocalCollection::deletePuzzle(const QModelIndex& index)
{
	//get puzzle object
	QObject* puzzlePayload = index.data(PuzzleObjectRole).value<QObject*>();
	Palapeli::OldPuzzle* puzzle = qobject_cast<Palapeli::OldPuzzle*>(puzzlePayload);
	if (!puzzle)
		return false;
	//check whether that particular file can be removed
	QString file = puzzle->location().toLocalFile();
	if (!QFile(file).remove())
		return false;
	//remove file from config
	KConfigGroup(m_group, puzzle->newPuzzle()->identifier()).deleteGroup();
	m_config->sync();
	//update internal storage
	removePuzzle(index);
	return true;
}

#include "collection-list.moc"

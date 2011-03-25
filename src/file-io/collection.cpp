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

#include "collection.h"
#include "collection_p.h"
#include "components.h"
#include "puzzle.h"

#include <QtCore/QFile>
#include <QtCore/QFutureWatcher>
#include <QtCore/QUuid>
#include <KDE/KConfig>
#include <KDE/KConfigGroup>
#include <KDE/KStandardDirs>

//BEGIN Palapeli::Collection::Item

Palapeli::Collection::Item::Item(Palapeli::Puzzle* puzzle)
	: m_puzzle(puzzle)
{
	//NOTE: Previously, the metadata.modifyProtection flag was used to decide
	//whether the puzzle is deletable. The current implementation uses the
	//identifier: Puzzles that have been imported by the user have UUID
	//identifiers. QUuid::createUuid().toString() always encloses the UUID in
	//curly braces.
	const QString id = puzzle->identifier();
	setData(id, Palapeli::Collection::IdentifierRole);
	setData(id.startsWith(QChar('{')), Palapeli::Collection::IsDeleteableRole);
	//request metadata
	Palapeli::FutureWatcher* watcher = new Palapeli::FutureWatcher;
	connect(watcher, SIGNAL(finished()), SLOT(populate()));
	connect(watcher, SIGNAL(finished()), watcher, SLOT(deleteLater()));
	watcher->setFuture(puzzle->get(Palapeli::PuzzleComponent::Metadata));
	//take ownership of puzzle
	m_puzzle->QObject::setParent(this);
}

void Palapeli::Collection::Item::populate()
{
	const Palapeli::MetadataComponent* cmp = m_puzzle->component<Palapeli::MetadataComponent>();
	if (!cmp)
		return;
	const Palapeli::PuzzleMetadata metadata = cmp->metadata;
	setData(metadata.name, Palapeli::Collection::NameRole);
	setData(metadata.comment, Palapeli::Collection::CommentRole);
	setData(metadata.author , Palapeli::Collection::AuthorRole);
	setData(metadata.pieceCount , Palapeli::Collection::PieceCountRole);
	setData(metadata.thumbnail , Palapeli::Collection::ThumbnailRole);
}

//END Palapeli::Collection::Item

Palapeli::Collection* Palapeli::Collection::instance()
{
	static Palapeli::Collection instance;
	return &instance;
}

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

Palapeli::Collection::Collection()
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
		Palapeli::Puzzle* puzzle = new Palapeli::Puzzle(new Palapeli::CollectionStorageComponent(puzzleGroup), url, puzzleId);
		appendRow(new Item(puzzle));
	}
}

Palapeli::Collection::~Collection()
{
	delete m_config;
	delete m_group;
}

Palapeli::Puzzle* Palapeli::Collection::puzzleFromIndex(const QModelIndex& index) const
{
	//a simple lookup like dynamic_cast<Item*>(itemFromIndex(index))->puzzle()
	//does not work because of proxy models in the CollectionView
	const QString identifier = index.data(IdentifierRole).toString();
	const int count = rowCount();
	for (int row = 0; row < count; ++row)
	{
		Item* item = dynamic_cast<Item*>(this->item(row));
		if (item && item->puzzle()->identifier() == identifier)
			return item->puzzle();
	}
	return 0;
}

void Palapeli::Collection::importPuzzle(Palapeli::Puzzle* puzzle)
{
	//determine new location
	const QString id = puzzle->identifier();
	const QString fileName = QString::fromLatin1("collection/%1.puzzle").arg(id);
	puzzle->setLocation(KUrl(KStandardDirs::locateLocal("appdata", fileName)));
	//store puzzle
	puzzle->get(Palapeli::PuzzleComponent::ArchiveStorage).waitForFinished();
	//create the config group for this puzzle (use pseudo-URL to avoid problems
	//when the configuration directory is moved)
	KConfigGroup puzzleGroup(m_group, id);
	puzzleGroup.writeEntry("Location", QString("palapeli:///%1").arg(fileName));
	m_config->sync();
	//add to the model
	appendRow(new Item(puzzle));
}

Palapeli::Puzzle* Palapeli::Collection::importPuzzle(const KUrl& location)
{
	const QString id = Palapeli::Puzzle::fsIdentifier(location);
	Palapeli::Puzzle* puzzle = new Palapeli::Puzzle(new Palapeli::ArchiveStorageComponent, location, id);
	//insert a copy of this puzzle into the collection
	const QString newId = QUuid::createUuid().toString();
	Palapeli::Puzzle* newPuzzle = new Palapeli::Puzzle(new Palapeli::CopyComponent(puzzle), location, newId);
	importPuzzle(newPuzzle);
	//cleanup
	puzzle->QObject::setParent(newPuzzle);
	return newPuzzle;
}

void Palapeli::Collection::exportPuzzle(const QModelIndex& index, const KUrl& location)
{
	//find existing puzzle
	Palapeli::Puzzle* puzzle = puzzleFromIndex(index);
	if (!puzzle)
		return;
	//create a copy of the given puzzle, and relocate it to the new location
	const QString identifier = Palapeli::Puzzle::fsIdentifier(location);
	Palapeli::Puzzle* newPuzzle = new Palapeli::Puzzle(new Palapeli::CopyComponent(puzzle), location, identifier);
	newPuzzle->get(Palapeli::PuzzleComponent::ArchiveStorage).waitForFinished();
}

bool Palapeli::Collection::deletePuzzle(const QModelIndex& index)
{
	Palapeli::Puzzle* puzzle = puzzleFromIndex(index);
	if (!puzzle)
		return false;
	//check whether that particular file can be removed
	QString file = puzzle->location().toLocalFile();
	if (!QFile(file).remove())
		return false;
	//remove puzzle from config
	KConfigGroup(m_group, puzzle->identifier()).deleteGroup();
	m_config->sync();
	//remove puzzle from model and delete it
	const int count = rowCount();
	for (int row = 0; row < count; ++row)
	{
		Item* item = dynamic_cast<Item*>(this->item(row));
		if (item && item->puzzle() == puzzle)
			qDeleteAll(this->takeRow(row));
	}
	return true;
}

#include "collection.moc"
#include "collection_p.moc"

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

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUuid>
#include <QStandardPaths>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

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
	setData(i18n("Loading puzzle..."), Qt::DisplayRole);
	setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	//request metadata
	puzzle->get(Palapeli::PuzzleComponent::Metadata);
	populate ();
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
	setData(metadata.author, Palapeli::Collection::AuthorRole);
	setData(metadata.pieceCount, Palapeli::Collection::PieceCountRole);
	setData(metadata.thumbnail, Palapeli::Collection::ThumbnailRole);
}

//END Palapeli::Collection::Item

Palapeli::Collection* Palapeli::Collection::instance()
{
	static Palapeli::Collection instance;
	return &instance;
}

static QString readPseudoUrl(const QString& path_, bool local)
{
	static const QLatin1String pseudoUrl("palapeli:/");
	if (path_.startsWith(pseudoUrl))
	{
		const QString path = path_.mid(pseudoUrl.size() + 1);
		if (local)
		{
			// this file can exist, if not, make sure all is ready
			// for creating the file
			// --> simulate KStandardDirs::locateLocal()
			const QString loc =
					QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) +
					path;
			QFileInfo fi(loc);
			if (fi.exists())
				return loc;
			// file does not exist, make sure directory exists
			QDir d(fi.absoluteDir());
			if (!d.exists())
				d.mkpath(fi.absolutePath());
			return loc;
		}
		else
		{
			// this file *must* exist
			return QStandardPaths::locate(QStandardPaths::AppDataLocation,
										  path,
										  QStandardPaths::LocateFile);
		}
	}
	else
		return path_;
}

Palapeli::Collection::Collection()
	: m_config(new KConfig(QStringLiteral("palapeli-collectionrc"), KConfig::CascadeConfig))
	, m_group(new KConfigGroup(m_config, "Palapeli Collection"))
{
	//read the puzzles
	const QStringList puzzleIds = m_group->groupList();
	foreach (const QString& puzzleId, puzzleIds)
	{
		KConfigGroup* puzzleGroup = new KConfigGroup(m_group, puzzleId);
		//find involved files
		const QString basePath = puzzleGroup->readEntry("Location", QString());
		const QString path = readPseudoUrl(basePath, false);
		QString baseDesktopPath(basePath);
		baseDesktopPath.replace(QRegExp("\\.puzzle$"), QLatin1String(".desktop"));
		const QString desktopPath = readPseudoUrl(baseDesktopPath, false);
		//construct puzzle with CollectionStorageComponent
		if (!path.isEmpty() && (desktopPath.isEmpty() || QFileInfo(path).lastModified() >= QFileInfo(desktopPath).lastModified()))
		{
			Palapeli::Puzzle* puzzle = new Palapeli::Puzzle(new Palapeli::CollectionStorageComponent(puzzleGroup, &m_configMutex), path, puzzleId);
			appendRow(new Item(puzzle));
			continue;
		}
		//no success - try to construct with RetailStorageComponent
		if (desktopPath.isEmpty())
			continue;
		const QString puzzlePath = readPseudoUrl(basePath, true);
		Palapeli::Puzzle* puzzle = new Palapeli::Puzzle(new Palapeli::RetailStorageComponent(desktopPath), puzzlePath, puzzleId);
		appendRow(new Item(puzzle));
		delete puzzleGroup;
		//make sure puzzle gets converted to archive format
		puzzle->get(Palapeli::PuzzleComponent::ArchiveStorage);
	}
	/* Moved out of CollectionStorageComponent, where we'd potentially call fdatasync on every
	   puzzle.  */
	m_configMutex.lock();
	m_config->sync();
	m_configMutex.unlock();
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
	const QString palapeliUrl = QStringLiteral("palapeli:///collection/%1.puzzle").arg(id);
	puzzle->setLocation(readPseudoUrl(palapeliUrl, true));
	//store puzzle
	puzzle->get(Palapeli::PuzzleComponent::ArchiveStorage);
	//create the config group for this puzzle (use pseudo-URL to avoid problems
	//when the configuration directory is moved)
	m_configMutex.lock();
	KConfigGroup puzzleGroup(m_group, id);
	puzzleGroup.writeEntry("Location", palapeliUrl);
	m_config->sync();
	m_configMutex.unlock();
	//add to the model
	appendRow(new Item(puzzle));
}

Palapeli::Puzzle* Palapeli::Collection::importPuzzle(const QString& path)
{
	const QString id = Palapeli::Puzzle::fsIdentifier(path);
	Palapeli::Puzzle* puzzle = new Palapeli::Puzzle(new Palapeli::ArchiveStorageComponent, path, id);
	//insert a copy of this puzzle into the collection
	const QString newId = QUuid::createUuid().toString();
	Palapeli::Puzzle* newPuzzle = new Palapeli::Puzzle(new Palapeli::CopyComponent(puzzle), path, newId);
	importPuzzle(newPuzzle);
	//cleanup
	puzzle->QObject::setParent(newPuzzle);
	return newPuzzle;
}

void Palapeli::Collection::exportPuzzle(const QModelIndex& index, const QString& path)
{
	//find existing puzzle
	Palapeli::Puzzle* puzzle = puzzleFromIndex(index);
	if (!puzzle)
		return;
	//create a copy of the given puzzle, and relocate it to the new location
	const QString identifier = Palapeli::Puzzle::fsIdentifier(path);
	Palapeli::Puzzle* newPuzzle = new Palapeli::Puzzle(new Palapeli::CopyComponent(puzzle), path, identifier);
	newPuzzle->get(Palapeli::PuzzleComponent::ArchiveStorage);
}

bool Palapeli::Collection::deletePuzzle(const QModelIndex& index)
{
	Palapeli::Puzzle* puzzle = puzzleFromIndex(index);
	if (!puzzle)
		return false;
	//check whether that particular file can be removed
	if (!QFile(puzzle->location()).remove())
		return false;
	//remove puzzle from config
	m_configMutex.lock();
	KConfigGroup(m_group, puzzle->identifier()).deleteGroup();
	m_config->sync();
	m_configMutex.unlock();
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


//

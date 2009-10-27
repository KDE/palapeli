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
#include "puzzle.h"

#include <QBuffer>
#include <QFileInfo>
#include <QUuid>
#include <KConfig>
#include <KConfigGroup>
#include <KIO/FileCopyJob>
#include <KIO/Job>
#include <KLocalizedString>
#include <KStandardDirs>
#include <KTemporaryFile>

//TODO: move the code that handles palapeli:/// URLs into LibraryCollection

Palapeli::ListCollection::ListCollection()
	: m_config(0)
{
}

Palapeli::ListCollection::ListCollection(const KUrl& url)
	: m_config(0)
{
	if (url.isLocalFile())
		setConfig(new KConfig(url.path(), KConfig::SimpleConfig));
	else
	{
		KTemporaryFile* tempFile = new KTemporaryFile;
		tempFile->QObject::setParent(this); //do not delete the temporary file until this collection is destroyed
		KIO::FileCopyJob* job = KIO::file_copy(url, KUrl(tempFile->fileName()));
		connect(job, SIGNAL(result(KJob*)), this, SLOT(collectionDataCopyFinished(KJob*)));
	}
}

Palapeli::ListCollection::~ListCollection()
{
	delete m_config;
}

void Palapeli::ListCollection::collectionDataCopyFinished(KJob* job)
{
	if (job->error())
		static_cast<KIO::Job*>(job)->showErrorDialog();
	else
		setConfig(new KConfig(static_cast<KIO::FileCopyJob*>(job)->destUrl().path(), KConfig::SimpleConfig));
}

KUrl Palapeli::ListCollection::readUrl(const KUrl& url) const
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

void Palapeli::ListCollection::setConfig(KConfig* config)
{
	if (m_config)
		return; //This method may only be called once.
	m_config = config;
	//start to read config
	KConfigGroup mainGroup(m_config, "Palapeli Collection");
	const QString name = mainGroup.readEntry("Name", QString());
	setName(name == "__my__" ? i18n("My collection") : name);
	m_features = mainGroup.readEntry("Features", QStringList());
	//read the puzzles
	const QStringList puzzleIds = mainGroup.groupList();
	foreach (const QString& puzzleId, puzzleIds)
	{
		KConfigGroup puzzleGroup(&mainGroup, puzzleId);
		//construct puzzle
		KUrl url = readUrl(puzzleGroup.readEntry("Location", KUrl()));
		if (url.isEmpty())
			continue;
		Palapeli::Puzzle* puzzle = new Palapeli::Puzzle(url);
		addPuzzleInternal(puzzle, puzzleId);
	}
}

QModelIndex Palapeli::ListCollection::addPuzzleInternal(Palapeli::Puzzle* puzzle, const QString& identifier)
{
	KConfigGroup mainGroup(m_config, "Palapeli Collection");
	KConfigGroup puzzleGroup(&mainGroup, identifier);
	//read metadata (we use the "Name" key to test whether metadata is available)
	//TODO: respect the ModifyDateTime key in the puzzleGroup in this step
	const QString name = puzzleGroup.readEntry("Name", QString());
	if (name.isEmpty())
	{
		//fill cache
		bool hasMetadata = (bool) puzzle->metadata();
		if (!hasMetadata)
			hasMetadata = puzzle->readMetadata();
		if (hasMetadata && m_features.contains("writecache"))
		{
			puzzleGroup.writeEntry("Name", puzzle->metadata()->name);
			puzzleGroup.writeEntry("Comment", puzzle->metadata()->comment);
			puzzleGroup.writeEntry("Author", puzzle->metadata()->author);
			puzzleGroup.writeEntry("PieceCount", puzzle->metadata()->pieceCount);
			//NOTE: Writing the thumbnail is a bit harder, because KConfig does not support images directly.
			QBuffer buffer;
			puzzle->metadata()->thumbnail.save(&buffer, "PNG");
			puzzleGroup.writeEntry("Thumbnail", buffer.data());
			m_config->sync();
		}
	}
	else if (m_features.contains("readcache"))
	{
		Palapeli::PuzzleMetadata* metadata = new Palapeli::PuzzleMetadata;
		metadata->name = puzzleGroup.readEntry("Name", QString());
		metadata->comment = puzzleGroup.readEntry("Comment", QString());
		metadata->author = puzzleGroup.readEntry("Author", QString());
		metadata->pieceCount = puzzleGroup.readEntry("PieceCount", 0);
		//NOTE: Reading the thumbnail is a bit harder, because KConfig does not support images directly.
		QImage image; image.loadFromData(puzzleGroup.readEntry("Thumbnail", QByteArray()));
		metadata->thumbnail = image;
		puzzle->injectMetadata(metadata);
	}
	return addPuzzle(puzzle, identifier);
}

QModelIndex Palapeli::ListCollection::storeGeneratedPuzzle(Palapeli::Puzzle* puzzle)
{
	if (!puzzle)
		return QModelIndex();
	if (!puzzle->location().isEmpty())
		return QModelIndex();
	return importPuzzleInternal(puzzle);
}

bool Palapeli::ListCollection::canImportPuzzles() const
{
	return m_features.contains("importpuzzle");
}

QModelIndex Palapeli::ListCollection::importPuzzle(const Palapeli::Puzzle* const puzzle)
{
	if (!puzzle->metadata())
		return QModelIndex();
	//create a writable copy of the given puzzle, and import this into the library
	Palapeli::Puzzle* newPuzzle = new Palapeli::Puzzle(*puzzle);
	return importPuzzleInternal(newPuzzle);
}

QModelIndex Palapeli::ListCollection::importPuzzleInternal(Palapeli::Puzzle* puzzle)
{
	//determine location of new puzzle
	const QString identifier = QUuid::createUuid().toString();
	const QString fileName = QString("puzzlelibrary/%1.puzzle").arg(identifier);
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
	return addPuzzleInternal(puzzle, identifier);
}

bool Palapeli::ListCollection::canDeletePuzzle(const QModelIndex& index) const
{
	if (!m_features.contains("removepuzzle"))
		return false;
	//get puzzle object
	QObject* puzzlePayload = index.data(PuzzleObjectRole).value<QObject*>();
	Palapeli::Puzzle* puzzle = qobject_cast<Palapeli::Puzzle*>(puzzlePayload);
	if (!puzzle)
		return false;
	//check whether that particular file can be removed
	QString file = puzzle->location().path();
	return QFileInfo(file).isWritable();
	//NOTE: This is a protection for the default puzzles, which are installed with read-only permissions.
}

bool Palapeli::ListCollection::deletePuzzle(const QModelIndex& index)
{
	if (!m_features.contains("removepuzzle"))
		return false;
	//get puzzle object
	QObject* puzzlePayload = index.data(PuzzleObjectRole).value<QObject*>();
	Palapeli::Puzzle* puzzle = qobject_cast<Palapeli::Puzzle*>(puzzlePayload);
	if (!puzzle)
		return false;
	//check whether that particular file can be removed
	QString file = puzzle->location().path();
	if (!QFile(file).remove())
		return false;
	//remove file from config
	KConfigGroup mainGroup(m_config, "Palapeli Collection");
	const QString identifier = index.data(IdentifierRole).toString();
	KConfigGroup(&mainGroup, identifier).deleteGroup();
	m_config->sync();
	//update internal storage
	removePuzzle(index);
	return true;
}

Palapeli::LibraryCollection::LibraryCollection()
{
	setConfig(new KConfig("palapeli-libraryrc", KConfig::CascadeConfig));
}

#include "collection-list.moc"

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
#include <KConfig>
#include <KConfigGroup>
#include <KIO/FileCopyJob>
#include <KIO/Job>
#include <KLocalizedString>
#include <KStandardDirs>
#include <KTemporaryFile>

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
		Palapeli::Puzzle* puzzle = new Palapeli::Puzzle(url);
		//read metadata (we use the "Name" key to test whether metadata is available)
		//TODO: respect the ModifyDateTime key in the puzzleGroup in this step
		const QString name = puzzleGroup.readEntry("Name", QString());
		if (name.isEmpty())
		{
			//fill cache
			if (puzzle->readMetadata() && m_features.contains("writecache"))
			{
				puzzleGroup.writeEntry("Name", puzzle->metadata()->name);
				puzzleGroup.writeEntry("Comment", puzzle->metadata()->comment);
				puzzleGroup.writeEntry("Author", puzzle->metadata()->author);
				puzzleGroup.writeEntry("PieceCount", puzzle->metadata()->pieceCount);
				//NOTE: Writing the thumbnail is a bit harder, because KConfig does not support images directly.
				QBuffer buffer;
				puzzle->metadata()->thumbnail.toImage().save(&buffer, "PNG");
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
			metadata->thumbnail = QPixmap::fromImage(image);
			puzzle->injectMetadata(metadata);
		}
		addPuzzle(puzzle, puzzleId);
	}
}

Palapeli::ListCollection::~ListCollection()
{
	delete m_config;
}

bool Palapeli::ListCollection::canImportPuzzles() const
{
	return m_features.contains("importpuzzle");
}

bool Palapeli::ListCollection::importPuzzle(Palapeli::Puzzle* puzzle)
{
	return false; //TODO: Palapeli::ListCollection::importPuzzle
}

bool Palapeli::ListCollection::canDeletePuzzle(const QModelIndex& index) const
{
	if (m_features.contains("removepuzzle"))
		return false; //TODO: Palapeli::ListCollection::canDeletePuzzle
	else
		return false;
}

bool Palapeli::ListCollection::deletePuzzle(const QModelIndex& index)
{
	return false; //TODO: Palapeli::ListCollection::deletePuzzle
}

Palapeli::LibraryCollection::LibraryCollection()
{
	setConfig(new KConfig("palapeli-libraryrc", KConfig::CascadeConfig));
}

#include "collection-list.moc"

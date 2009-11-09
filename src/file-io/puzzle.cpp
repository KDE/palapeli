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

#include "puzzle.h"

#include <QtConcurrentRun>
#include <KConfigGroup>
#include <KDesktopFile>
#include <KIO/FileCopyJob>
#include <KIO/Job>
#include <KIO/NetAccess>
#include <KTar>
#include <KTempDir>
#include <KTemporaryFile>

const QSize Palapeli::Puzzle::ThumbnailBaseSize(64, 64);

Palapeli::Puzzle::Puzzle(const KUrl& location)
	: m_location(location)
	, m_loadLocation(location)
	, m_metadata(0)
	, m_contents(0)
	, m_creationContext(0)
	, m_cache(0)
{
	connect(&m_createArchiveWatcher, SIGNAL(finished()), this, SLOT(finishWritingArchive()));
}

Palapeli::Puzzle::Puzzle(const Palapeli::Puzzle& other)
	: QObject()
	, m_location(other.m_location)
	, m_loadLocation(other.m_loadLocation)
	, m_metadata(0)
	, m_contents(0)
	, m_creationContext(0)
	, m_cache(0)
{
	connect(&m_createArchiveWatcher, SIGNAL(finished()), this, SLOT(finishWritingArchive()));
	if (other.m_metadata)
		m_metadata = new Palapeli::PuzzleMetadata(*other.m_metadata);
	if (other.m_contents)
		m_contents = new Palapeli::PuzzleContents(*other.m_contents);
}

Palapeli::Puzzle::Puzzle(Palapeli::PuzzleMetadata* metadata, Palapeli::PuzzleContents* contents, Palapeli::PuzzleCreationContext* creationContext)
	: m_metadata(metadata)
	, m_contents(contents)
	, m_creationContext(creationContext)
	, m_cache(0)
{
	connect(&m_createArchiveWatcher, SIGNAL(finished()), this, SLOT(finishWritingArchive()));
}

Palapeli::Puzzle::~Puzzle()
{
	delete m_metadata;
	delete m_contents;
}

KUrl Palapeli::Puzzle::location() const
{
	return m_location;
}

void Palapeli::Puzzle::setLocation(const KUrl& location)
{
	m_location = location;
}

const Palapeli::PuzzleMetadata* Palapeli::Puzzle::metadata() const
{
	return m_metadata;
}

const Palapeli::PuzzleContents* Palapeli::Puzzle::contents() const
{
	return m_contents;
}

void Palapeli::Puzzle::injectMetadata(Palapeli::PuzzleMetadata* metadata)
{
	delete m_metadata;
	m_metadata = metadata;
}

bool Palapeli::Puzzle::readMetadata(bool force)
{
	if (m_metadata && m_cache && !force)
		return true; //nothing to do (puzzle has been loaded from URL, and cache is already filled)
	if (m_metadata && m_contents && m_loadLocation.isEmpty())
		return true; //nothing to do (puzzle has just been created, and filling the cache could be harmful)
	delete m_metadata; m_metadata = 0;
	delete m_cache; m_cache = 0;
	//make archive available locally
	QString archiveFile;
	if (!m_loadLocation.isLocalFile())
	{
		if (!KIO::NetAccess::download(m_loadLocation, archiveFile, 0))
			return false;
	}
	else
		archiveFile = m_loadLocation.path();
	//open archive and extract into temporary directory
	KTar tar(archiveFile, "application/x-gzip");
	if (!tar.open(QIODevice::ReadOnly))
	{
		KIO::NetAccess::removeTempFile(archiveFile);
		return false;
	}
	const KArchiveDirectory* archiveDir = tar.directory();
	m_cache = new KTempDir;
	const QString cachePath = m_cache->name(); //note: includes trailing slash
	archiveDir->copyTo(cachePath);
	tar.close();
	//cleanup
	KIO::NetAccess::removeTempFile(archiveFile);
	//read metadata
	KDesktopFile manifest(m_cache->name() + "pala.desktop");
	m_metadata = new Palapeli::PuzzleMetadata;
	m_metadata->name = manifest.readName();
	m_metadata->author = manifest.desktopGroup().readEntry("X-KDE-PluginInfo-Author", QString());
	m_metadata->comment = manifest.readComment();
	m_metadata->image.load(m_cache->name() + "image.jpg");
	m_metadata->thumbnail = m_metadata->image.scaled(ThumbnailBaseSize, Qt::KeepAspectRatio);
	//find piece count
	KConfigGroup offsetGroup(&manifest, "PieceOffsets");
	QList<QString> offsetGroupKeys = offsetGroup.entryMap().keys();
	m_metadata->pieceCount = 0;
	while (offsetGroupKeys.contains(QString::number(m_metadata->pieceCount)))
		++m_metadata->pieceCount;
	return true;
}

bool Palapeli::Puzzle::readContents(bool force)
{
	if (m_contents && !force)
		return true; //nothing to do
	if (!readMetadata(true)) //try to load puzzle file
		return false;
	delete m_contents; m_contents = new Palapeli::PuzzleContents;
	//load more metadata
	KDesktopFile manifest(m_cache->name() + "pala.desktop");
	m_contents->imageSize = KConfigGroup(&manifest, "Job").readEntry("ImageSize", QSize());
	//load piece offsets
	KConfigGroup offsetGroup(&manifest, "PieceOffsets");
	QList<QString> offsetGroupKeys = offsetGroup.entryMap().keys();
	foreach (const QString& offsetGroupKey, offsetGroupKeys)
	{
		bool ok = true;
		const int pieceIndex = offsetGroupKey.toInt(&ok);
		if (ok)
			m_contents->pieceOffsets[pieceIndex] = offsetGroup.readEntry(offsetGroupKey, QPoint());
	}
	//load pieces
	QList<int> pieceIDs = m_contents->pieceOffsets.keys();
	foreach (int pieceID, pieceIDs)
		m_contents->pieces[pieceID].load(m_cache->name() + QString("%1.png").arg(pieceID));
	//load relations
	KConfigGroup relationsGroup(&manifest, "Relations");
	for (int index = 0;; ++index)
	{
		QList<int> value = relationsGroup.readEntry(QString::number(index), QList<int>());
		if (value.isEmpty()) //end of relations list
			break;
		m_contents->relations << QPair<int, int>(value[0], value[1]);
	}
	return !m_contents->pieces.isEmpty();
}

bool Palapeli::Puzzle::write()
{
	//optimization: if nothing has changed since the puzzle has been loaded, just copy the original puzzle file to the new location
	if (!m_loadLocation.isEmpty())
	{
		KIO::FileCopyJob* job = KIO::file_copy(m_loadLocation, m_location);
		connect(job, SIGNAL(result(KJob*)), this, SLOT(writeFinished(KJob*)));
		return true;
	}
	//the complex write operation (the one that creates and fills a new cache, and writes a new manifest) is being run in a separate thread
	if (!m_metadata || !m_contents)
		return false; //not enough data available for this operation
	//createNewArchiveFile can only be called in separate thread if creation context is available (otherwise, we have to use the piece pixmaps from PuzzleContents, which cannot be used in a non-GUI thread)
	if (m_creationContext)
		m_createArchiveWatcher.setFuture(QtConcurrent::run(this, &Palapeli::Puzzle::createNewArchiveFile));
	else
	{
		createNewArchiveFile();
		finishWritingArchive();
	}
	//in general, we don't know better and have to assume that Palapeli::Puzzle::createNewArchiveFile does not fail
	return true;
}

void Palapeli::Puzzle::writeFinished(KJob* job)
{
	if (job->error())
		static_cast<KIO::Job*>(job)->showErrorDialog();
	else
	{
		if (m_location.isLocalFile())
		{
			QFile file(m_location.path());
			file.setPermissions(file.permissions() | QFile::WriteOwner | QFile::WriteGroup); //make file deleteable
		}
		emit writeFinished();
	}
}

void Palapeli::Puzzle::createNewArchiveFile()
{
	delete m_cache; m_cache = new KTempDir;
	const QString cachePath = m_cache->name();
	//write manifest
	KConfig manifest(cachePath + "pala.desktop");
	KConfigGroup mainGroup(&manifest, "Desktop Entry");
	mainGroup.writeEntry("Name", m_metadata->name);
	mainGroup.writeEntry("Comment", m_metadata->comment);
	mainGroup.writeEntry("X-KDE-PluginInfo-Author", m_metadata->author);
	mainGroup.writeEntry("Type", "X-Palapeli-Puzzle");
	KConfigGroup jobGroup(&manifest, "Job");
	jobGroup.writeEntry("ImageSize", m_contents->imageSize);
	if (m_creationContext)
	{
		jobGroup.writeEntry("Image", KUrl("kfiledialog:///palapeli/pseudopath")); //just a placeholder, to make sure that an "Image" key is available
		jobGroup.writeEntry("Slicer", m_creationContext->usedSlicer);
		QMapIterator<QByteArray, QVariant> iterSlicerArgs(m_creationContext->usedSlicerArgs);
		while (iterSlicerArgs.hasNext())
		{
			iterSlicerArgs.next();
			jobGroup.writeEntry(QString::fromUtf8(iterSlicerArgs.key()), iterSlicerArgs.value());
		}
	}
	//write pieces to cache
	if (m_creationContext)
	{
		QMapIterator<int, QImage> iterPieces(m_creationContext->pieces);
		while (iterPieces.hasNext())
		{
			const QString imagePath = cachePath + QString::fromLatin1("%1.png").arg(iterPieces.next().key());
			if (!iterPieces.value().save(imagePath))
			{
				delete m_cache;
				return;
			}
		}
	}
	else
	{
		QMapIterator<int, QPixmap> iterPieces(m_contents->pieces);
		while (iterPieces.hasNext())
		{
			const QString imagePath = cachePath + QString::fromLatin1("%1.png").arg(iterPieces.next().key());
			if (!iterPieces.value().save(imagePath))
			{
				delete m_cache;
				return;
			}
		}
	}
	//write thumbnail into tempdir
	const QString imagePath = cachePath + QString::fromLatin1("image.jpg");
	if (!m_metadata->image.save(imagePath))
		return;
	//write piece offsets into target manifest
	KConfigGroup offsetGroup(&manifest, "PieceOffsets");
	QMapIterator<int, QPoint> iterOffsets(m_contents->pieceOffsets);
	while (iterOffsets.hasNext())
	{
		iterOffsets.next();
		offsetGroup.writeEntry(QString::number(iterOffsets.key()), iterOffsets.value());
	}
	//write piece relations into target manifest
	KConfigGroup relationsGroup(&manifest, "Relations");
	for (int index = 0; index < m_contents->relations.count(); ++index)
	{
		const QPair<int, int> relation = m_contents->relations[index];
		relationsGroup.writeEntry(QString::number(index), QList<int>() << relation.first << relation.second);
	}
	//save manifest
	manifest.sync();
}

void Palapeli::Puzzle::finishWritingArchive()
{
	//compress archive to temporary file
	KTemporaryFile* tempFile = new KTemporaryFile;
	tempFile->setSuffix(".puzzle");
	tempFile->open();
	KTar tar(tempFile->fileName(), "application/x-gzip");
	if (!tar.open(QIODevice::WriteOnly))
		return;
	else if (!tar.addLocalDirectory(m_cache->name(), QLatin1String(".")))
		return;
	else if (!tar.close())
		return;
	//upload puzzle file to m_location
	KIO::FileCopyJob* job = KIO::file_copy(KUrl(tempFile->fileName()), m_location);
	connect(job, SIGNAL(result(KJob*)), this, SLOT(writeFinished(KJob*)));
	tempFile->QObject::setParent(job); //tempfile can safely be deleted after copy job is finished
	//NOTE: Above code is written in such a way that a boolean return value can be added to this method later without big efforts.
	//puzzle is now available at m_location
	m_loadLocation = m_location;
}

#include "puzzle.moc"

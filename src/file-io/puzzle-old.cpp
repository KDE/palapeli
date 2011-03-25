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

#include "puzzle-old.h"
#include "puzzle.h"
#include "components.h"

#include <QtConcurrentRun>
#include <KConfigGroup>
#include <KDesktopFile>
#include <KIO/JobUiDelegate>
#include <KIO/FileCopyJob>
#include <KIO/Job>
#include <KIO/NetAccess>
#include <KTar>
#include <KTempDir>
#include <KTemporaryFile>

const QSize Palapeli::PuzzleMetadata::ThumbnailBaseSize(64, 64);

Palapeli::OldPuzzle::OldPuzzle(const KUrl& location)
	: m_location(location)
	, m_loadLocation(location)
	, m_puzzle(new Palapeli::Puzzle(new Palapeli::ArchiveStorageComponent(location)))
	, m_metadata(0)
	, m_contents(0)
	, m_creationContext(0)
	, m_cache(0)
{
	connect(&m_createArchiveWatcher, SIGNAL(finished()), this, SLOT(finishWritingArchive()));
}

Palapeli::OldPuzzle::OldPuzzle(const Palapeli::OldPuzzle& other)
	: QObject()
	, m_location(other.m_location)
	, m_loadLocation(other.m_loadLocation)
	, m_puzzle(new Palapeli::Puzzle(new Palapeli::ArchiveStorageComponent(m_location)))
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

Palapeli::OldPuzzle::OldPuzzle(const Palapeli::PuzzleCreationContext& creationContext)
	: m_puzzle(new Palapeli::Puzzle(new Palapeli::CreationContextComponent(creationContext)))
	, m_metadata(0)
	, m_contents(0)
	, m_creationContext(new Palapeli::PuzzleCreationContext(creationContext))
	, m_cache(0)
{
	connect(&m_createArchiveWatcher, SIGNAL(finished()), this, SLOT(finishWritingArchive()));
	readMetadata();
	readContents();
}

Palapeli::OldPuzzle::~OldPuzzle()
{
	delete m_puzzle;
	delete m_metadata;
	delete m_contents;
}

KUrl Palapeli::OldPuzzle::location() const
{
	return m_location;
}

void Palapeli::OldPuzzle::setLocation(const KUrl& location)
{
	m_location = location;
}

Palapeli::Puzzle* Palapeli::OldPuzzle::newPuzzle() const
{
	return m_puzzle;
}

const Palapeli::PuzzleMetadata* Palapeli::OldPuzzle::metadata() const
{
	return m_metadata;
}

const Palapeli::PuzzleContents* Palapeli::OldPuzzle::contents() const
{
	return m_contents;
}

void Palapeli::OldPuzzle::injectMetadata(Palapeli::PuzzleMetadata* metadata)
{
	delete m_metadata;
	m_metadata = metadata;
}

bool Palapeli::OldPuzzle::readMetadata()
{
	if (m_metadata)
		return true;
	const Palapeli::PuzzleComponent* component = m_puzzle->get(Palapeli::PuzzleComponent::Metadata);
	if (!component)
		return false;
	m_metadata = new Palapeli::PuzzleMetadata(dynamic_cast<const Palapeli::MetadataComponent*>(component)->metadata);
	return true;
}

bool Palapeli::OldPuzzle::readContents()
{
	if (!readMetadata()) //legacy code might rely on metadata being available after readContents()
		return false;
	if (m_contents)
		return true;
	const Palapeli::PuzzleComponent* component = m_puzzle->get(Palapeli::PuzzleComponent::Contents);
	if (!component)
		return false;
	m_contents = new Palapeli::PuzzleContents(dynamic_cast<const Palapeli::ContentsComponent*>(component)->contents);
	return true;
}

bool Palapeli::OldPuzzle::write()
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
		m_createArchiveWatcher.setFuture(QtConcurrent::run(this, &Palapeli::OldPuzzle::createNewArchiveFile));
	else
	{
		createNewArchiveFile();
		finishWritingArchive();
	}
	//in general, we don't know better and have to assume that Palapeli::OldPuzzle::createNewArchiveFile does not fail
	return true;
}

void Palapeli::OldPuzzle::writeFinished(KJob* job)
{
	if (job->error())
		static_cast<KIO::Job*>(job)->ui()->showErrorMessage();
	else
	{
		if (m_location.isLocalFile())
		{
			QFile file(m_location.toLocalFile());
			file.setPermissions(file.permissions() | QFile::WriteOwner | QFile::WriteGroup); //make file deleteable
		}
		emit writeFinished();
	}
}

void Palapeli::OldPuzzle::createNewArchiveFile()
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
	KConfigGroup collectionGroup(&manifest, "Collection");
	collectionGroup.writeEntry("ModifyProtection", m_metadata->modifyProtection);
	KConfigGroup jobGroup(&manifest, "Job");
	jobGroup.writeEntry("ImageSize", m_contents->imageSize);
	if (m_creationContext)
	{
		jobGroup.writeEntry("Image", KUrl("kfiledialog:///palapeli/pseudopath")); //just a placeholder, to make sure that an "Image" key is available
		jobGroup.writeEntry("Slicer", m_creationContext->slicer);
		jobGroup.writeEntry("SlicerMode", m_creationContext->slicerMode);
		QMapIterator<QByteArray, QVariant> iterSlicerArgs(m_creationContext->slicerArgs);
		while (iterSlicerArgs.hasNext())
		{
			iterSlicerArgs.next();
			jobGroup.writeEntry(QString::fromUtf8(iterSlicerArgs.key()), iterSlicerArgs.value());
		}
	}
	//write pieces to cache
	QMapIterator<int, QImage> iterPieces(m_contents->pieces);
	while (iterPieces.hasNext())
	{
		const QString imagePath = cachePath + QString::fromLatin1("%1.png").arg(iterPieces.next().key());
		if (!iterPieces.value().save(imagePath))
		{
			delete m_cache;
			return;
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

void Palapeli::OldPuzzle::finishWritingArchive()
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

#include "puzzle-old.moc"

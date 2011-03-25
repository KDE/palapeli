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

Palapeli::OldPuzzle::OldPuzzle(Palapeli::PuzzleComponent* mainComponent, const KUrl& location, const QString& identifier)
	: m_puzzle(new Palapeli::Puzzle(mainComponent, location, identifier))
	, m_cache(0)
{
	connect(&m_createArchiveWatcher, SIGNAL(finished()), this, SLOT(finishWritingArchive()));
}

Palapeli::OldPuzzle::OldPuzzle(const KUrl& location, const QString& identifier)
	: m_puzzle(new Palapeli::Puzzle(new Palapeli::ArchiveStorageComponent, location, identifier))
	, m_cache(0)
{
	connect(&m_createArchiveWatcher, SIGNAL(finished()), this, SLOT(finishWritingArchive()));
}

Palapeli::OldPuzzle::OldPuzzle(const Palapeli::OldPuzzle& other, const QString& identifier)
	: QObject()
	, m_puzzle(new Palapeli::Puzzle(new Palapeli::CopyComponent(other.m_puzzle), other.m_puzzle->location(), identifier))
	, m_cache(0)
{
	connect(&m_createArchiveWatcher, SIGNAL(finished()), this, SLOT(finishWritingArchive()));
}

Palapeli::OldPuzzle::OldPuzzle(const Palapeli::PuzzleCreationContext& creationContext, const QString& identifier)
	: m_puzzle(new Palapeli::Puzzle(new Palapeli::CreationContextComponent(creationContext), KUrl(), identifier))
	, m_cache(0)
{
	connect(&m_createArchiveWatcher, SIGNAL(finished()), this, SLOT(finishWritingArchive()));
	readMetadata();
	readContents();
}

Palapeli::OldPuzzle::~OldPuzzle()
{
	delete m_puzzle;
}

KUrl Palapeli::OldPuzzle::location() const
{
	return m_puzzle->location();
}

void Palapeli::OldPuzzle::setLocation(const KUrl& location)
{
	m_puzzle->setLocation(location);
}

Palapeli::Puzzle* Palapeli::OldPuzzle::newPuzzle() const
{
	return m_puzzle;
}

const Palapeli::PuzzleMetadata* Palapeli::OldPuzzle::metadata() const
{
	const Palapeli::MetadataComponent* c = m_puzzle->component<Palapeli::MetadataComponent>();
	return c ? &c->metadata : 0;
}

const Palapeli::PuzzleContents* Palapeli::OldPuzzle::contents() const
{
	const Palapeli::ContentsComponent* c = m_puzzle->component<Palapeli::ContentsComponent>();
	return c ? &c->contents : 0;
}

bool Palapeli::OldPuzzle::readMetadata()
{
	const Palapeli::PuzzleComponent* component = m_puzzle->get(Palapeli::PuzzleComponent::Metadata);
	return (bool) component;
}

bool Palapeli::OldPuzzle::readContents()
{
	if (!readMetadata()) //legacy code might rely on metadata being available after readContents()
		return false;
	const Palapeli::PuzzleComponent* component = m_puzzle->get(Palapeli::PuzzleComponent::Contents);
	return (bool) component;
}

bool Palapeli::OldPuzzle::write()
{
	//the complex write operation (the one that creates and fills a new cache, and writes a new manifest) is being run in a separate thread
	const Palapeli::PuzzleComponent* cMetadata = m_puzzle->get(Palapeli::PuzzleComponent::Metadata);
	const Palapeli::PuzzleComponent* cContents = m_puzzle->get(Palapeli::PuzzleComponent::Contents);
	m_puzzle->get(Palapeli::PuzzleComponent::CreationContext).waitForFinished();
	if (!cMetadata || !cContents)
		return false; //not enough data available for this operation
	m_createArchiveWatcher.setFuture(QtConcurrent::run(this, &Palapeli::OldPuzzle::createNewArchiveFile));
	//in general, we don't know better and have to assume that Palapeli::OldPuzzle::createNewArchiveFile does not fail
	return true;
}

void Palapeli::OldPuzzle::createNewArchiveFile()
{
	const Palapeli::PuzzleMetadata metadata = m_puzzle->component<Palapeli::MetadataComponent>()->metadata;
	const Palapeli::PuzzleContents contents = m_puzzle->component<Palapeli::ContentsComponent>()->contents;
	//create cache
	delete m_cache; m_cache = new KTempDir;
	const QString cachePath = m_cache->name();
	//write manifest
	KConfig manifest(cachePath + "pala.desktop");
	KConfigGroup mainGroup(&manifest, "Desktop Entry");
	mainGroup.writeEntry("Name", metadata.name);
	mainGroup.writeEntry("Comment", metadata.comment);
	mainGroup.writeEntry("X-KDE-PluginInfo-Author", metadata.author);
	mainGroup.writeEntry("Type", "X-Palapeli-Puzzle");
	KConfigGroup collectionGroup(&manifest, "Collection");
	collectionGroup.writeEntry("ModifyProtection", metadata.modifyProtection);
	KConfigGroup jobGroup(&manifest, "Job");
	jobGroup.writeEntry("ImageSize", contents.imageSize);
	if (m_puzzle->component<Palapeli::CreationContextComponent>())
	{
		const Palapeli::PuzzleCreationContext creationContext = m_puzzle->component<Palapeli::CreationContextComponent>()->creationContext;
		jobGroup.writeEntry("Image", KUrl("kfiledialog:///palapeli/pseudopath")); //just a placeholder, to make sure that an "Image" key is available
		jobGroup.writeEntry("Slicer", creationContext.slicer);
		jobGroup.writeEntry("SlicerMode", creationContext.slicerMode);
		QMapIterator<QByteArray, QVariant> iterSlicerArgs(creationContext.slicerArgs);
		while (iterSlicerArgs.hasNext())
		{
			iterSlicerArgs.next();
			jobGroup.writeEntry(QString::fromUtf8(iterSlicerArgs.key()), iterSlicerArgs.value());
		}
	}
	//write pieces to cache
	QMapIterator<int, QImage> iterPieces(contents.pieces);
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
	if (!metadata.image.save(imagePath))
		return;
	//write piece offsets into target manifest
	KConfigGroup offsetGroup(&manifest, "PieceOffsets");
	QMapIterator<int, QPoint> iterOffsets(contents.pieceOffsets);
	while (iterOffsets.hasNext())
	{
		iterOffsets.next();
		offsetGroup.writeEntry(QString::number(iterOffsets.key()), iterOffsets.value());
	}
	//write piece relations into target manifest
	KConfigGroup relationsGroup(&manifest, "Relations");
	for (int index = 0; index < contents.relations.count(); ++index)
	{
		const QPair<int, int> relation = contents.relations[index];
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
	KIO::FileCopyJob* job = KIO::file_copy(KUrl(tempFile->fileName()), m_puzzle->location());
	connect(job, SIGNAL(result(KJob*)), this, SLOT(writeFinished(KJob*)));
	tempFile->QObject::setParent(job); //tempfile can safely be deleted after copy job is finished
	//NOTE: Above code is written in such a way that a boolean return value can be added to this method later without big efforts.
}

void Palapeli::OldPuzzle::writeFinished(KJob* job)
{
	if (job->error())
		static_cast<KIO::Job*>(job)->ui()->showErrorMessage();
	else
	{
		const KUrl location = m_puzzle->location();
		if (location.isLocalFile())
		{
			QFile file(location.toLocalFile());
			file.setPermissions(file.permissions() | QFile::WriteOwner | QFile::WriteGroup); //make file deleteable
		}
		emit writeFinished();
	}
}

#include "puzzle-old.moc"

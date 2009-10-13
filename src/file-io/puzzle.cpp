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

#include <KConfigGroup>
#include <KDesktopFile>
#include <KIO/FileCopyJob>
#include <KIO/Job>
#include <KIO/NetAccess>
#include <KTar>
#include <KTempDir>

const QSize Palapeli::Puzzle::ThumbnailBaseSize(64, 64);

Palapeli::Puzzle::Puzzle(const KUrl& location)
	: m_location(location)
	, m_loadLocation(location)
	, m_metadata(0)
	, m_contents(0)
	, m_cache(0)
{
}

Palapeli::Puzzle::Puzzle(const Palapeli::Puzzle& other)
	: QObject()
	, m_location(other.m_location)
	, m_loadLocation(other.m_loadLocation)
	, m_metadata(0)
	, m_contents(0)
	, m_cache(0)
{
	if (other.m_metadata)
		m_metadata = new Palapeli::PuzzleMetadata(*other.m_metadata);
	if (other.m_contents)
		m_contents = new Palapeli::PuzzleContents(*other.m_contents);
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
		return true; //nothing to do
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
	KTar tar(archiveFile, "application/x-bzip");
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
	m_metadata->thumbnail = QImage(m_cache->name() + "thumbnail.jpg").scaled(ThumbnailBaseSize, Qt::KeepAspectRatio);
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
	//optimisation: if nothing has changed since the puzzle has been loaded, just copy the original puzzle file to the new location
	if (!m_loadLocation.isEmpty())
	{
		KIO::FileCopyJob* job = KIO::file_copy(m_loadLocation, m_location);
		connect(job, SIGNAL(result(KJob*)), this, SLOT(writeFinished(KJob*)));
		return true;
	}
	//TODO: Palapeli::Puzzle::write NOT IMPLEMENTED YET for new puzzles
	return false;
}

void Palapeli::Puzzle::writeFinished(KJob* job)
{
	if (job->error())
		static_cast<KIO::Job*>(job)->showErrorDialog();
}

#include "puzzle.moc"

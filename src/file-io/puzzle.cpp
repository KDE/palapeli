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

#include <QFileInfo>
#include <KConfigGroup>
#include <KDesktopFile>
#include <KIO/NetAccess>
#include <KStandardDirs>
#include <KTar>
#include <KTempDir>

const QSize Palapeli::Puzzle::ThumbnailBaseSize(64, 64);

Palapeli::Puzzle::Puzzle(const Palapeli::PuzzleLocation& location)
	: m_location(location)
	, m_metadata(0)
	, m_contents(0)
	, m_cache(0)
{
	if (location.isFromLibrary())
	{
		KConfig cache(KStandardDirs::locateLocal("appdata", "puzzlelibrary/cache.conf"));
		KConfigGroup cacheGroup(&cache, location.identifier());
		const QDateTime fileModifyDate = QFileInfo(location.url().path()).lastModified();
		//determine if cache can be used
		bool useCache = false;
		if (cache.hasGroup(location.identifier()))
			useCache = fileModifyDate <= cacheGroup.readEntry("ModifyDateTime", QDateTime());
		static const QString thumbnailPathTemplate = QString::fromLatin1("puzzlelibrary/%1.png");
		const QString thumbnailPath = thumbnailPathTemplate.arg(location.identifier());
		if (useCache)
		{
			//load metadata from cache
			m_metadata = new Palapeli::PuzzleMetadata;
			m_metadata->name = cacheGroup.readEntry("Name", QString());
			m_metadata->author = cacheGroup.readEntry("Author", QString());
			m_metadata->comment = cacheGroup.readEntry("Comment", QString());
			m_metadata->pieceCount = cacheGroup.readEntry("PieceCount", 0);
			m_metadata->thumbnail.load(KStandardDirs::locate("appdata", thumbnailPath));
		}
		else
		{
			//load metadata from archive
			readMetadata();
			//write metadata into cache
			cacheGroup.writeEntry("ModifyDateTime", fileModifyDate);
			cacheGroup.writeEntry("Name", m_metadata->name);
			cacheGroup.writeEntry("Author", m_metadata->author);
			cacheGroup.writeEntry("Comment", m_metadata->comment);
			cacheGroup.writeEntry("PieceCount", m_metadata->pieceCount);
			m_metadata->thumbnail.save(KStandardDirs::locateLocal("appdata", thumbnailPath));
			cache.sync();
		}
	}
}

Palapeli::Puzzle::Puzzle(const Palapeli::Puzzle& other)
	: m_location(other.m_location)
	, m_metadata(new Palapeli::PuzzleMetadata(*other.m_metadata))
	, m_contents(new Palapeli::PuzzleContents(*other.m_contents))
	, m_cache(0)
{
}

Palapeli::Puzzle::~Puzzle()
{
	delete m_metadata;
	delete m_contents;
}

Palapeli::PuzzleLocation Palapeli::Puzzle::location() const
{
	return m_location;
}

void Palapeli::Puzzle::setLocation(const Palapeli::PuzzleLocation& location)
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

bool Palapeli::Puzzle::readMetadata(bool force)
{
	if (m_metadata && !force)
		return true; //nothing to do
	delete m_metadata; m_metadata = 0;
	delete m_cache; m_cache = 0;
	//make archive available locally
	QString archiveFile;
	if (!m_location.url().isLocalFile())
	{
		if (!KIO::NetAccess::download(m_location.url(), archiveFile, 0))
			return false;
	}
	else
		archiveFile = m_location.url().path();
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
	m_metadata->thumbnail = QPixmap(m_cache->name() + "thumbnail.jpg").scaled(ThumbnailBaseSize, Qt::KeepAspectRatio);
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
	if (!m_cache)
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
	return false; //TODO: Palapeli::Puzzle::write NOT IMPLEMENTED YET
}

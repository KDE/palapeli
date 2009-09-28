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

#include "puzzlereader.h"

#include <QFileInfo>
#include <KConfigGroup>
#include <KDesktopFile>
#include <KIO/NetAccess>
#include <KStandardDirs>
#include <KTar>
#include <KTempDir>

const QSize Palapeli::PuzzleReader::ThumbnailBaseSize(64, 64);

KUrl urlForLibPuzzle(const QString& identifier)
{
	static const QString pathTemplate = QString::fromLatin1("palapeli/puzzlelibrary/%1.pala");
	return KStandardDirs::locate("data", pathTemplate.arg(identifier));
}

Palapeli::PuzzleReader::PuzzleReader(const QString& identifier)
	: m_locationUrl(urlForLibPuzzle(identifier))
	, m_metadataLoaded(false)
	, m_puzzleLoaded(false)
	, m_cache(0)
	, m_manifest(0)
{
	KConfig cache(KStandardDirs::locateLocal("data", "palapeli/puzzlelibrary/cache.conf"));
	KConfigGroup cacheGroup(&cache, identifier);
	const QDateTime fileModifyDate = QFileInfo(m_locationUrl.path()).lastModified();
	//determine if cache can be used
	bool loadMetadataFromArchive = true;
	if (cache.hasGroup(identifier))
		loadMetadataFromArchive = fileModifyDate > cacheGroup.readEntry("ModifyDateTime", QDateTime());
	static const QString thumbnailPathTemplate = QString::fromLatin1("palapeli/puzzlelibrary/%1.png");
	if (loadMetadataFromArchive)
	{
		//load metadata from archive
		loadMetadata();
		//write metadata into cache
		cacheGroup.writeEntry("ModifyDateTime", fileModifyDate);
		cacheGroup.writeEntry("Name", m_name);
		cacheGroup.writeEntry("Author", m_author);
		cacheGroup.writeEntry("Comment", m_comment);
		cacheGroup.writeEntry("PieceCount", m_pieceCount);
		m_thumbnail.save(KStandardDirs::locateLocal("data", thumbnailPathTemplate.arg(identifier)));
		cache.sync();
	}
	else
	{
		//load metadata from cache
		KConfigGroup cacheGroup(&cache, identifier);
		m_name = cacheGroup.readEntry("Name", QString());
		m_author = cacheGroup.readEntry("Author", QString());
		m_comment = cacheGroup.readEntry("Comment", QString());
		m_pieceCount = cacheGroup.readEntry("PieceCount", 0);
		m_thumbnail.load(KStandardDirs::locate("data", thumbnailPathTemplate.arg(identifier)));
		m_metadataLoaded = true;
	}
}

Palapeli::PuzzleReader::PuzzleReader(const KUrl& locationUrl)
	: m_locationUrl(locationUrl)
	, m_metadataLoaded(false)
	, m_puzzleLoaded(false)
	, m_cache(0)
	, m_manifest(0)
{
}

Palapeli::PuzzleReader::~PuzzleReader()
{
	delete m_cache;
	delete m_manifest;
}

void Palapeli::PuzzleReader::loadArchive()
{
	if (m_cache)
		return;
	//make archive available locally
	QString archiveFile;
	if (!m_locationUrl.isLocalFile())
	{
		if (!KIO::NetAccess::download(m_locationUrl, archiveFile, 0))
			return;
	}
	else
		archiveFile = m_locationUrl.path();
	//open archive and extract into temporary directory
	KTar tar(archiveFile, "application/x-bzip");
	if (!tar.open(QIODevice::ReadOnly))
	{
		KIO::NetAccess::removeTempFile(archiveFile);
		return;
	}
	const KArchiveDirectory* archiveDir = tar.directory();
	m_cache = new KTempDir;
	const QString cachePath = m_cache->name(); //note: includes trailing slash
	archiveDir->copyTo(cachePath);
	tar.close();
	//cleanup
	KIO::NetAccess::removeTempFile(archiveFile);
}

void Palapeli::PuzzleReader::loadMetadata()
{
	if (m_metadataLoaded)
		return;
	m_metadataLoaded = true;
	//load archive if necessary
	if (!m_cache)
		loadArchive();
	if (!m_cache)
		return;
	//load manifest and thumbnail
	m_manifest = new KDesktopFile(m_cache->name() + "pala.desktop");
	m_name = m_manifest->readName();
	m_author = m_manifest->desktopGroup().readEntry("X-KDE-PluginInfo-Author", QString());
	m_comment = m_manifest->readComment();
	m_thumbnail = QPixmap(m_cache->name() + "thumbnail.jpg").scaled(ThumbnailBaseSize, Qt::KeepAspectRatio);
	//find piece count
	KConfigGroup offsetGroup(m_manifest, "PieceOffsets");
	QList<QString> offsetGroupKeys = offsetGroup.entryMap().keys();
	m_pieceCount = 0;
	while (offsetGroupKeys.contains(QString::number(m_pieceCount)))
		++m_pieceCount;
}

void Palapeli::PuzzleReader::loadPuzzle()
{
	if (m_puzzleLoaded)
		return;
	m_puzzleLoaded = true;
	//load archive and manifest if necessary
	if (!m_cache)
		loadArchive();
	if (!m_manifest)
		m_manifest = new KDesktopFile(m_cache->name() + "pala.desktop");
	//load more metadata
	m_imageSize = KConfigGroup(m_manifest, "Job").readEntry("ImageSize", QSize());
	//load piece offsets
	KConfigGroup offsetGroup(m_manifest, "PieceOffsets");
	QList<QString> offsetGroupKeys = offsetGroup.entryMap().keys();
	foreach (const QString& offsetGroupKey, offsetGroupKeys)
	{
		bool ok = true;
		const int pieceIndex = offsetGroupKey.toInt(&ok);
		if (ok)
			m_pieceOffsets[pieceIndex] = offsetGroup.readEntry(offsetGroupKey, QPoint());
	}
	//load pieces
	QList<int> pieceIDs = m_pieceOffsets.keys();
	foreach (int pieceID, pieceIDs)
		m_pieces[pieceID].load(m_cache->name() + QString("%1.png").arg(pieceID));
	//load relations
	KConfigGroup relationsGroup(m_manifest, "Relations");
	for (int index = 0;; ++index)
	{
		QList<int> value = relationsGroup.readEntry(QString::number(index), QList<int>());
		if (value.isEmpty()) //end of relations list
			break;
		m_relations << QPair<int, int>(value[0], value[1]);
	}
}

QString Palapeli::PuzzleReader::identifier() const
{
	QString fileName = m_locationUrl.fileName(KUrl::IgnoreTrailingSlash);
	return fileName.section('.', 0, 0);
}

QString Palapeli::PuzzleReader::name() const
{
	return m_name;
}

QString Palapeli::PuzzleReader::author() const
{
	return m_author;
}

QString Palapeli::PuzzleReader::comment() const
{
	return m_comment;
}

int Palapeli::PuzzleReader::pieceCount() const
{
	return m_pieceCount;
}

QPixmap Palapeli::PuzzleReader::thumbnail() const
{
	return m_thumbnail;
}

QSize Palapeli::PuzzleReader::imageSize() const
{
	return m_imageSize;
}

QMap<int, QPixmap> Palapeli::PuzzleReader::pieces() const
{
	return m_pieces;
}

QMap<int, QPoint> Palapeli::PuzzleReader::pieceOffsets() const
{
	return m_pieceOffsets;
}

QList<QPair<int, int> > Palapeli::PuzzleReader::relations() const
{
	return m_relations;
}

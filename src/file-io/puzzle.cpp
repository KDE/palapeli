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
#include <KIO/NetAccess>
#include <KTar>
#include <KTempDir>
#include <KTemporaryFile>

Palapeli::Puzzle::Puzzle(const KUrl& locationUrl)
	: m_locationUrl(locationUrl)
	, m_cache(0)
	, m_manifest(0)
{
}

Palapeli::Puzzle::~Puzzle()
{
	delete m_cache;
	delete m_manifest;
}

QString Palapeli::Puzzle::identifier() const
{
	QString fileName = m_locationUrl.fileName(KUrl::IgnoreTrailingSlash);
	return fileName.section('.', 0, 0);
}

const KDesktopFile* Palapeli::Puzzle::manifest()
{
	if (!m_manifest)
		loadPuzzleContents();
	return m_manifest;
}

QMap<int, QPixmap> Palapeli::Puzzle::pieces()
{
	if (m_pieces.isEmpty())
		loadPuzzleContents();
	return m_pieces;
}

QMap<int, QPoint> Palapeli::Puzzle::pieceOffsets()
{
	if (m_pieceOffsets.isEmpty())
		loadPuzzleContents();
	return m_pieceOffsets;
}

QList<QPair<int, int> > Palapeli::Puzzle::relations()
{
	if (m_relations.isEmpty())
		loadPuzzleContents();
	return m_relations;
}

void Palapeli::Puzzle::loadArchive()
{
	//clear caches
	delete m_cache;
	m_cache = 0;
	delete m_manifest;
	m_manifest = 0;
	m_pieces.clear();
	m_pieceOffsets.clear();
	m_relations.clear();
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

void Palapeli::Puzzle::loadPuzzleContents()
{
	if (!m_cache)
		loadArchive();
	if (!m_cache) //could not load archive
		return;
	//clear caches
	delete m_manifest;
	m_manifest = 0;
	m_pieces.clear();
	m_pieceOffsets.clear();
	m_relations.clear();
	//load manifest
	m_manifest = new KDesktopFile(m_cache->name() + "pala.desktop");
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

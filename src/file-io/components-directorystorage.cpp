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

#include "components.h"

#include <QtCore/QDir>
#include <KDE/KConfigGroup>
#include <KDE/KDesktopFile>
#include <KDE/KTempDir>

Palapeli::DirectoryStorageComponent::DirectoryStorageComponent()
	: m_dir(new KTempDir)
{
}

Palapeli::DirectoryStorageComponent::~DirectoryStorageComponent()
{
	m_dir->unlink();
	delete m_dir;
}

QString Palapeli::DirectoryStorageComponent::directory() const
{
	return m_dir->name();
}

Palapeli::PuzzleComponent* Palapeli::DirectoryStorageComponent::cast(Palapeli::PuzzleComponent::Type type) const
{
	QDir dir(m_dir->name());
	//load metadata from directory
	if (type == Palapeli::PuzzleComponent::Metadata)
	{
		Palapeli::PuzzleMetadata metadata;
		KDesktopFile manifest(dir.absoluteFilePath("pala.desktop"));
		//read simple metadata
		metadata.name = manifest.readName();
		metadata.author = manifest.desktopGroup().readEntry("X-KDE-PluginInfo-Author", QString());
		metadata.comment = manifest.readComment();
		metadata.modifyProtection = manifest.group("Collection").readEntry("ModifyProtection", false);
		//read image
		metadata.image.load(dir.absoluteFilePath("image.jpg"));
		metadata.thumbnail = metadata.image.scaled(Palapeli::PuzzleMetadata::ThumbnailBaseSize, Qt::KeepAspectRatio);
		//count pieces
		const QStringList keys = manifest.group("PieceOffsets").keyList();
		metadata.pieceCount = 0;
		while (keys.contains(QString::number(metadata.pieceCount)))
			++metadata.pieceCount;
		//done
		return new Palapeli::MetadataComponent(metadata);
	}
	//load contents from directory
	else if (type == Palapeli::PuzzleComponent::Contents)
	{
		Palapeli::PuzzleContents contents;
		KDesktopFile manifest(dir.absoluteFilePath("pala.desktop"));
		contents.imageSize = manifest.group("Job").readEntry("ImageSize", QSize());
		//load piece offsets and images
		KConfigGroup offsetGroup(&manifest, "PieceOffsets");
		const QStringList offsetKeys = offsetGroup.keyList();
		for (int i = 0; i < offsetKeys.count(); ++i)
		{
			const QString key = offsetKeys[i];
			bool ok = true;
			const int pieceIndex = key.toInt(&ok);
			if (!ok)
				continue;
			contents.pieceOffsets[pieceIndex] = offsetGroup.readEntry(key, QPoint());
			contents.pieces[pieceIndex].load(dir.absoluteFilePath(key + ".png"));
		}
		//load relations
		KConfigGroup relationsGroup(&manifest, "Relations");
		for (int i = 0;; ++i)
		{
			QList<int> value = relationsGroup.readEntry(QString::number(i), QList<int>());
			if (value.size() < 2) //end of relations list
				break;
			contents.relations << QPair<int, int>(value[0], value[1]);
		}
		//done
		return new Palapeli::ContentsComponent(contents);
	}
	//TODO: load creation context from directory
	//TODO: compress directory into file (after making sure that metadata && (contents || creationContext) are available)
	return 0;
}

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
#include <QtCore/QFutureSynchronizer>
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
	if (type == Metadata)
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
	else if (type == Contents)
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
	//load creation context from directory
	else if (type == CreationContext)
	{
		puzzle()->get(Metadata).waitForFinished();
		const Palapeli::PuzzleMetadata metadata = puzzle()->component<Palapeli::MetadataComponent>()->metadata;
		//initialize creation context from existing metadata
		Palapeli::PuzzleCreationContext creationContext;
		*((Palapeli::PuzzleMetadata*)&creationContext) = metadata;
		//load slicer configuration
		KDesktopFile manifest(dir.absoluteFilePath("pala.desktop"));
		KConfigGroup jobGroup(&manifest, "Job");
		creationContext.slicer = jobGroup.readEntry("Slicer", QString());
		creationContext.slicerMode = jobGroup.readEntry("SlicerMode", QByteArray());
		//all the other entries in jobGroup belong into slicerArgs
		QMap<QString, QString> args = jobGroup.entryMap();
		args.remove(QLatin1String("Image"));
		args.remove(QLatin1String("ImageSize"));
		args.remove(QLatin1String("Slicer"));
		args.remove(QLatin1String("SlicerMode"));
		QMapIterator<QString, QString> iter(args);
		while (iter.hasNext())
		{
			iter.next();
			creationContext.slicerArgs.insert(iter.key().toUtf8(), iter.value());
		}
		return new Palapeli::CreationContextComponent(creationContext);
	}
	//compress directory into file
	else if (type == DirectoryStorage)
		return Palapeli::DirectoryStorageComponent::fromData(puzzle());
	else
		return 0;
}

Palapeli::DirectoryStorageComponent* Palapeli::DirectoryStorageComponent::fromData(Palapeli::Puzzle* puzzle)
{
	//make sure that everything's available
	QFutureSynchronizer<void> sync;
	sync.addFuture(puzzle->get(Metadata));
	sync.addFuture(puzzle->get(Contents));
	sync.addFuture(puzzle->get(CreationContext));
	sync.waitForFinished();
	//retrieve data (only metadata and contents are totally necessary)
	const Palapeli::MetadataComponent* cMetadata = puzzle->component<Palapeli::MetadataComponent>();
	const Palapeli::ContentsComponent* cContents = puzzle->component<Palapeli::ContentsComponent>();
	if (!cMetadata || !cContents)
		return 0;
	const Palapeli::CreationContextComponent* cCreationContext = puzzle->component<Palapeli::CreationContextComponent>();
	const Palapeli::PuzzleMetadata metadata = cMetadata->metadata;
	const Palapeli::PuzzleContents contents = cContents->contents;
	//create cache
	Palapeli::DirectoryStorageComponent* cmp = new Palapeli::DirectoryStorageComponent;
	QDir dir(cmp->directory());
	//write manifest
	KConfig manifest(dir.absoluteFilePath("pala.desktop"));
	KConfigGroup mainGroup(&manifest, "Desktop Entry");
	mainGroup.writeEntry("Name", metadata.name);
	mainGroup.writeEntry("Comment", metadata.comment);
	mainGroup.writeEntry("X-KDE-PluginInfo-Author", metadata.author);
	mainGroup.writeEntry("Type", "X-Palapeli-Puzzle");
	KConfigGroup collectionGroup(&manifest, "Collection");
	collectionGroup.writeEntry("ModifyProtection", metadata.modifyProtection);
	KConfigGroup jobGroup(&manifest, "Job");
	jobGroup.writeEntry("ImageSize", contents.imageSize);
	if (cCreationContext)
	{
		const Palapeli::PuzzleCreationContext creationContext = cCreationContext->creationContext;
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
		const QString imagePath = dir.absoluteFilePath(QString::fromLatin1("%1.png").arg(iterPieces.next().key()));
		iterPieces.value().save(imagePath);
	}
	//write thumbnail into tempdir
	const QString imagePath = dir.absoluteFilePath(QLatin1String("image.jpg"));
	metadata.image.save(imagePath);
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
	//save manifest; done
	manifest.sync();
	return cmp;
}

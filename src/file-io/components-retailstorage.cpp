/*
    SPDX-FileCopyrightText: 2011 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "components.h"
#include "puzzle.h"
#include "puzzlestructs.h"

#include <QDir>
#include <KConfigGroup>
#include <KDesktopFile>

Palapeli::RetailStorageComponent::RetailStorageComponent(const QString& desktopFile)
	: m_desktopFile(desktopFile)
{
}

Palapeli::PuzzleComponent* Palapeli::RetailStorageComponent::cast(Type type) const
{
	//all casting via CreationContextComponent
	if (type != CreationContext)
	{
		return puzzle()->get(CreationContext)->cast(type);
	}
	//create creation context
	Palapeli::PuzzleCreationContext creationContext;
	//open manifest
	QDir dir = QFileInfo(m_desktopFile).dir();
	KDesktopFile manifest(m_desktopFile);
	KConfigGroup jobGroup(&manifest, "Job");
	//read simple metadata
	creationContext.name = manifest.readName();
	creationContext.author = manifest.desktopGroup().readEntry("X-KDE-PluginInfo-Author", QString());
	creationContext.comment = manifest.readComment();
	creationContext.modifyProtection = manifest.group("Collection").readEntry("ModifyProtection", false);
	//read image
	const QString imageName = jobGroup.readEntry("Image", QString());
	creationContext.image.load(dir.absoluteFilePath(imageName));
	creationContext.thumbnail = creationContext.image.scaled(Palapeli::PuzzleMetadata::ThumbnailBaseSize, Qt::KeepAspectRatio);
	creationContext.pieceCount = -1; //TODO: not available now
	//load slicer configuration
	creationContext.slicer = jobGroup.readEntry("Slicer", QString());
	creationContext.slicerMode = jobGroup.readEntry("SlicerMode", QByteArray());
	//all the other entries in jobGroup belong into slicerArgs
	QMap<QString, QString> args = jobGroup.entryMap();
	args.remove(QStringLiteral("Image"));
	args.remove(QStringLiteral("ImageSize"));
	args.remove(QStringLiteral("Slicer"));
	args.remove(QStringLiteral("SlicerMode"));
	QMapIterator<QString, QString> iter(args);
	while (iter.hasNext())
	{
		iter.next();
		creationContext.slicerArgs.insert(iter.key().toUtf8(), iter.value());
	}
	return new Palapeli::CreationContextComponent(creationContext);
}

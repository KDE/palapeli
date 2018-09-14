/***************************************************************************
 *   Copyright 2011 Stefan Majewsky <majewsky@gmx.net>
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
#include "puzzle.h"
#include "puzzlestructs.h"

#include <QDir>
#include <QVariant>
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
		return puzzle()->get(CreationContext).result()->cast(type);
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

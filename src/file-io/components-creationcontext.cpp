/*
    SPDX-FileCopyrightText: 2009-2011 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "components.h"

#include <Pala/Slicer>
#include <Pala/SlicerJob>
#include <Pala/SlicerMode>

#include <KPluginFactory>
#include <KPluginMetaData>
#include <KPluginLoader>

Palapeli::PuzzleComponent* Palapeli::CreationContextComponent::cast(Type type) const
{
	//just a short-hand
	const Palapeli::PuzzleCreationContext& cc = creationContext;
	//metadata is in creationContext (except for piece count which needs to be
	//inferred from contents)
	if (type == Palapeli::PuzzleComponent::Metadata)
	{
		Palapeli::PuzzleMetadata metadata(cc);
		const Palapeli::PuzzleComponent* cmp = puzzle()->get(Contents);
		const Palapeli::ContentsComponent* cmp2 = dynamic_cast<const Palapeli::ContentsComponent*>(cmp);
		metadata.pieceCount = cmp2 ? cmp2->contents.pieces.count() : -1;
		return new Palapeli::MetadataComponent(metadata);
	}
	//contents can be built
	else if (type == Palapeli::PuzzleComponent::Contents)
	{
		//TODO: move slicer instantiation to a location that is shared between
		//      puzzle creator dialog and this function
		//find slicer
		const QVector<KPluginMetaData> offers = KPluginLoader::findPlugins(QStringLiteral("palapelislicers"), [cc](const KPluginMetaData &m) {
			return m.pluginId() == cc.slicer;
		});
		if (offers.isEmpty())
		{
			CAST_ERROR(QString::fromLatin1("Could not find slicer \"%1\".").arg(cc.slicer));
			return nullptr;
		}
		//initialize requested slicer plugin
		KPluginLoader loader(offers.first().fileName());
		KPluginFactory *factory = loader.factory();
		QScopedPointer<Pala::Slicer> slicer(factory->create<Pala::Slicer>(nullptr, QVariantList()));
		if (!slicer)
		{
			CAST_ERROR(QString::fromLatin1("Could not load slicer \"%1\": %2").arg(cc.slicer).arg(loader.errorString()));
			return nullptr;
		}
		//create job
		Pala::SlicerJob job(cc.image, cc.slicerArgs);
		if (!cc.slicerMode.isEmpty())
		{
			const auto modes = slicer->modes();
			for (const Pala::SlicerMode* mode : modes)
				if (mode->key() == cc.slicerMode)
				{
					job.setMode(mode);
					break;
				}
			if (!job.mode())
			{
				CAST_ERROR(QString::fromLatin1("Could not find slicer mode \"%1\".").arg(QString::fromUtf8(cc.slicerMode)));
				return nullptr;
			}
		}
		//do slicing
		if (!slicer->process(&job))
		{
			CAST_ERROR(QString::fromLatin1("Slicing failed because of undetermined problems."));
			return nullptr;
		}
		//assemble PuzzleContents
		Palapeli::PuzzleContents contents;
		contents.imageSize = job.image().size();
		contents.pieces = job.pieces();
		contents.pieceOffsets = job.pieceOffsets();
		contents.relations = job.relations();
		return new Palapeli::ContentsComponent(contents);
	}
	//casts for writing an archive
	else if (type == DirectoryStorage)
		return Palapeli::DirectoryStorageComponent::fromData(puzzle());
	else if (type == ArchiveStorage)
		return Palapeli::ArchiveStorageComponent::fromData(puzzle());
	//unknown type requested
	else
		return nullptr;
}

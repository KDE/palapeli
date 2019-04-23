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
#include "../libpala/slicer.h"
#include "../libpala/slicerjob.h"
#include "../libpala/slicermode.h"

#include <KServiceTypeTrader>

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
		KService::List offers = KServiceTypeTrader::self()->query(QStringLiteral("Libpala/SlicerPlugin"));
		KService::Ptr slicerOffer;
		foreach (KService::Ptr offer, offers)
			if (offer->library() == cc.slicer)
			{
				slicerOffer = offer;
				break;
			}
		if (!slicerOffer)
		{
			CAST_ERROR(QString::fromLatin1("Could not find slicer \"%1\".").arg(cc.slicer));
			return 0;
		}
		//initialize requested slicer plugin
		QString errorMessage;
		QScopedPointer<Pala::Slicer> slicer(slicerOffer->createInstance<Pala::Slicer>(0, QVariantList(), &errorMessage));
		if (!slicer)
		{
			CAST_ERROR(QString::fromLatin1("Could not load slicer \"%1\": %2").arg(cc.slicer).arg(errorMessage));
			return 0;
		}
		//create job
		Pala::SlicerJob job(cc.image, cc.slicerArgs);
		if (!cc.slicerMode.isEmpty())
		{
			foreach (const Pala::SlicerMode* mode, slicer->modes())
				if (mode->key() == cc.slicerMode)
				{
					job.setMode(mode);
					break;
				}
			if (!job.mode())
			{
				CAST_ERROR(QString::fromLatin1("Could not find slicer mode \"%1\".").arg(QString::fromUtf8(cc.slicerMode)));
				return 0;
			}
		}
		//do slicing
		if (!slicer->process(&job))
		{
			CAST_ERROR(QString::fromLatin1("Slicing failed because of undetermined problems."));
			return 0;
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
		return 0;
}

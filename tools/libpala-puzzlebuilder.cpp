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

#include "../libpala/slicer.h"
#include "../libpala/slicerjob.h"

#include <iostream>
#include <QFileInfo>
#include <QImage>
#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>
#include <KCmdLineOptions>
#include <KConfig>
#include <KConfigGroup>
#include <KServiceTypeTrader>
#include <KTar>
#include <KTempDir>

int main(int argc, char** argv)
{
	KAboutData about("libpala-puzzlebuilder", "palapeli", ki18nc("The application's name", "libpala-puzzlebuilder"), "0.0.1", ki18n("Automatic builder for Palapeli puzzles"), KAboutData::License_GPL, ki18n("Copyright 2009, Stefan Majewsky"));
	about.addAuthor(ki18n("Stefan Majewsky"), KLocalizedString(), "majewsky@gmx.net", "http://majewsky.wordpress.com");

	KCmdLineOptions options;
	options.add("+file", ki18nc("description for a command line switch", "Configuration file"));
	KCmdLineArgs::addCmdLineOptions(options);
	
	KCmdLineArgs::init(argc, argv, &about);
	KApplication app;
	KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

	if (args->count() < 2)
	{
		std::cerr << "Error: Too few arguments." << std::endl;
		return 1;
	}

	//get command line options, and check if all required options have been given
	const QString manifestPath = args->url(0).path();
	const QString outputPath = args->url(1).path();
	if (!QFileInfo(manifestPath).isFile())
	{
		std::cerr << "Error: Path to manifest invalid, or manifest file not found." << std::endl;
		return 1;
	}

	//load manifest
	KConfig manifest(manifestPath, KConfig::SimpleConfig);

	//read "Job" group
	QMap<QByteArray, QVariant> slicerArgs;
	QString imagePath;
	QString slicerName;

	QMap<QString, QString> jobData = KConfigGroup(&manifest, "Job").entryMap();
	QMapIterator<QString, QString> jobDataIterator(jobData);
	while (jobDataIterator.hasNext())
	{
		jobDataIterator.next();
		if (jobDataIterator.key() == QLatin1String("Image"))
			imagePath = jobDataIterator.value();
		else if (jobDataIterator.key() == QLatin1String("Slicer"))
			slicerName = jobDataIterator.value();
		else
			slicerArgs[jobDataIterator.key().toLatin1()] = QVariant::fromValue(jobDataIterator.value());
	}

	//load image
	QImage image(imagePath);
	if (image.size().isEmpty())
	{
		std::cerr << "Error: Image file inaccessible or corrupted." << std::endl;
		return 1;
	}

	//find requested slicer plugin
	KService::List offers = KServiceTypeTrader::self()->query("Libpala/SlicerPlugin");
	KService::Ptr slicerOffer;
	foreach (KService::Ptr offer, offers)
		if (offer->library() == slicerName)
			slicerOffer = offer;
	if (!slicerOffer)
	{
		std::cerr << "Error: Requested slicer not available." << std::endl;
		return 1;
	}

	//initialize requested slicer plugin
	QString errorMessage;
	Pala::Slicer* slicer = slicerOffer->createInstance<Pala::Slicer>(0, QVariantList(), &errorMessage);
	if (!errorMessage.isEmpty())
	{
		std::cerr << qPrintable(errorMessage) << std::endl;
		return 1;
	}

	//create slicer job
	Pala::SlicerJob job(image, slicerArgs);
	if (!slicer->run(&job))
	{
		std::cerr << "Error: Slicing failed." << std::endl;
		return 1;
	}

	//assemble everything into a KTempDir
	KTempDir cache;
	const QString cachePath = cache.name();
	//copy manifest to tempdir
	const QString targetManifestPath = cachePath + "pala.desktop";
	if (!QFile(manifestPath).copy(targetManifestPath))
	{
		std::cerr << "Manifest could not be copied into archive." << std::endl;
		return 1;
	}
	//copy pieces to tempdir
	QMap<int, QImage> pieces = job.pieces();
	QList<int> pieceIndices = pieces.keys();
	foreach (int index, pieceIndices)
	{
		if (!pieces[index].save(cachePath + QString("%1.jpg").arg(index)))
		{
			std::cerr << "Could not save piece image no. " << index << std::endl;
			return 1;
		}
	}
	//write piece offsets into target manifest
	QMap<int, QPoint> pieceOffsets = job.pieceOffsets();
	KConfig targetManifest(targetManifestPath, KConfig::SimpleConfig);
	KConfigGroup offsetGroup(&targetManifest, "PieceOffsets");
	foreach (int index, pieceIndices)
		offsetGroup.writeEntry(QString::number(index), pieceOffsets[index]);
	//write piece relations into target manifest
	QList<QPair<int, int> > relations = job.relations();
	KConfigGroup relationsGroup(&targetManifest, "Relations");
	for (int index = 0; index < relations.count(); ++index)
		relationsGroup.writeEntry(QString::number(index), QList<int>() << relations[index].first << relations[index].second);
	//write image size into target manifest (used to determine the size of the puzzle table)
	KConfigGroup jobGroup(&targetManifest, "Job");
	jobGroup.writeEntry("ImageSize", image.size());
	targetManifest.sync();

	//compress archive to temporary file
	KTar tar(outputPath, "application/x-bzip");
	bool success = true;
	if (!tar.open(QIODevice::WriteOnly))
		success = false;
	else if (!tar.addLocalDirectory(cachePath, QLatin1String(".")))
		success = false;
	else if (!tar.close())
		success = false;
	if (!success)
	{
		std::cerr << "Error: Could not write output file." << std::endl;
		return 1;
	}

	return 0;
}

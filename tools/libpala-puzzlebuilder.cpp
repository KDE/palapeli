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

#include <KDebug>

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

	//get command line options, and check if all required options have been given
	QString manifestPath;
	if (args->count() > 0)
		manifestPath = args->url(0).path();
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
	Palapeli::Slicer* slicer = slicerOffer->createInstance<Palapeli::Slicer>(0, QVariantList(), &errorMessage);
	if (!errorMessage.isEmpty())
	{
		std::cerr << errorMessage.toUtf8().data() << std::endl;
		return 1;
	}

	//create slicer job
	kDebug() << "Args" << slicerArgs;
	Palapeli::SlicerJob job(image, slicerArgs);
	slicer->run(&job);

	//DEBUG: write out images and offsets to test the slicing
	QMap<int, QImage> pieces = job.pieces();
	QMap<int, QPoint> pieceOffsets = job.pieceOffsets();
	foreach (int index, pieces.keys())
	{
		pieces[index].save(QString("%1.png").arg(index));
		kDebug() << QString("Piece %1:").arg(index) << "Size" << pieces[index].size() << "Offset" << pieceOffsets[index];
	}

	return 0;
}

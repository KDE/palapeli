/***************************************************************************
 *   Copyright (C) 2008 Felix Lemke <lemke.felix@ages-skripte.org>
 *   Copyright (C) 2008 Stefan Majewsky <majewsky@gmx.net>
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

#include "view.h"

#include <time.h>
#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>
#include <KCmdLineOptions>
#include <KGlobal>
#include <KLocale>
#include <KLocalizedString>

int main(int argc, char** argv)
{
	qsrand(time(0));

	KAboutData about("palapeli", "palapeli", ki18n("Palapeli"), "0.1", ki18n("A jigsaw puzzle game"), KAboutData::License_GPL, ki18n("(c) 2008, the Palapeli team"));
	about.addAuthor(ki18n("Felix Lemke"), KLocalizedString(), "lemke.felix@ages-skripte.org");
	about.addAuthor(ki18n("Stefan Majewsky"), KLocalizedString(), "majewsky@gmx.net");
	KCmdLineArgs::init(argc, argv, &about);

	KCmdLineOptions options;
	options.add("i");
	options.add("image <path>", ki18n("Generate puzzle from the given image"), "/usr/share/wallpapers/Water01.jpg");
	options.add("x");
	options.add("xcount <int>", ki18n("Number of pieces in X direction"), "10");
	options.add("y");
	options.add("ycount <int>", ki18n("Number of pieces in Y direction"), "10");
	options.add("w");
	options.add("width <pixels>", ki18n("Width of puzzle scene (defaults to the double image width)"), "-1");
	options.add("h");
	options.add("height <pixels>", ki18n("Height of puzzle scene (defaults to the double image height)"), "-1");
	options.add("", ki18n("The puzzle scene can be bigger as your monitor, you can scroll it.")); //a comment added below the options
	KCmdLineArgs::addCmdLineOptions(options);

	KApplication app;
    KGlobal::locale()->insertCatalog("libkdegames");

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	QString fileName = args->getOption("image");
	int xCount = args->getOption("xcount").toInt();
	if (xCount <= 0)
		xCount = 10;
	int yCount = args->getOption("ycount").toInt();
	if (yCount <= 0)
		yCount = 10;
	int sceneWidth = args->getOption("width").toInt();
	if (sceneWidth <= 0 && sceneWidth == -1)
		sceneWidth = -1;
	int sceneHeight = args->getOption("height").toInt();
	if (sceneHeight <= 0 && sceneWidth == -1)
		sceneHeight = -1;
	args->clear();

	Palapeli::View view;
	view.show();
	view.startGame(sceneWidth, sceneHeight, fileName, xCount, yCount);

	return app.exec();
}

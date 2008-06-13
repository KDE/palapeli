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

#include "manager.h"
#include "mainwindow.h"

#include <time.h>
#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>
//#include <KCmdLineOptions>
#include <KGlobal>
#include <KIcon>
#include <KLocale>
#include <KLocalizedString>

//TODO: Resurrect command line args: If an image is set, start a game automatically; -x and -y should be defaults for the "New puzzle" dialog.
//TODO: Import and export of savegames.
//TODO: Transform patterns into shared libraries which can be loaded dynamically from a .desktop database (like Plasma applets, hopefully including GHNS support.)

int main(int argc, char** argv)
{
	qsrand(time(0));

	KAboutData about("palapeli", "palapeli", ki18nc("The application's name", "Palapeli"), "0.1", ki18n("A jigsaw puzzle game"), KAboutData::License_GPL, ki18n("(c) 2008, the Palapeli team"));
	about.addAuthor(ki18n("Felix Lemke"), KLocalizedString(), "lemke.felix@ages-skripte.org");
	about.addAuthor(ki18n("Stefan Majewsky"), KLocalizedString(), "majewsky@gmx.net");
	KCmdLineArgs::init(argc, argv, &about);

//	KCmdLineOptions options;
//	options.add("i");
//	options.add("image <path>", ki18n("Generate puzzle from the given image"), "/usr/share/wallpapers/Water01.jpg");
//	options.add("x");
//	options.add("xcount <int>", ki18n("Number of pieces in X direction"), "10");
//	options.add("y");
//	options.add("ycount <int>", ki18n("Number of pieces in Y direction"), "10");
//	KCmdLineArgs::addCmdLineOptions(options);

	KApplication app;
	app.setWindowIcon(KIcon("preferences-plugin"));
	KGlobal::locale()->insertCatalog("libkdegames");

//	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
//	QString fileName = args->getOption("image");
//	int xCount = args->getOption("xcount").toInt();
//	if (xCount <= 0)
//		xCount = 10;
//	int yCount = args->getOption("ycount").toInt();
//	if (yCount <= 0)
//		yCount = 10;
//	args->clear();

	Palapeli::Manager manager;
	manager.window()->show();
	return app.exec();
}

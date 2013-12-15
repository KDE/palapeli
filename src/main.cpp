/***************************************************************************
 *   Copyright 2009, 2010 Stefan Majewsky <majewsky@gmx.net>
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

#include "importhelper.h"
#include "window/mainwindow.h"

#include <ctime>
#include <QTimer>
#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>

int main(int argc, char** argv)
{
	qsrand(time(0));
	KAboutData about("palapeli", 0, ki18nc("The application's name", "Palapeli"), "1.2", ki18n("KDE Jigsaw Puzzle Game"), KAboutData::License_GPL, ki18n("Copyright 2009, 2010, Stefan Majewsky"));
	about.addAuthor(ki18n("Stefan Majewsky"), KLocalizedString(), "majewsky@gmx.net", "http://majewsky.wordpress.com");
	about.addCredit (ki18n ("Johannes Loehnert"),
			 ki18n ("The option to preview the completed puzzle"),
			 "loehnert.kde@gmx.de");
	KCmdLineArgs::init(argc, argv, &about);

	KCmdLineOptions options;
	options.add("+puzzlefile", ki18n("Path to puzzle file (will be opened if -i is not given)"));
	options.add("i").add("import", ki18n("Import the given puzzle file into the local collection (does nothing if no puzzle file is given)"));
	options.add("", ki18n("If the -i/--import option is specified, the main window will not be shown after importing the given puzzle."));
	KCmdLineArgs::addCmdLineOptions(options);

	KApplication app;

	KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
	//NOTE: Syntax errors are reported on stderr, while file errors are presented to the user.
	if (args->isSet("import"))
		//perform import request
		new Palapeli::ImportHelper(args);
	else
		//no import request, show main window
		(new Palapeli::MainWindow(args))->show();
	return app.exec();
}

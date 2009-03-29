/***************************************************************************
 *   Copyright 2008 Felix Lemke <lemke.felix@ages-skripte.org>
 *   Copyright 2008 Stefan Majewsky <majewsky@gmx.net>
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

#include "main.h"
#include "manager.h"

#include <ctime>
#include <KApplication>
#include <KCmdLineArgs>
#include <KGlobal>
#include <KIcon>
#include <KLocale>

int main(int argc, char** argv)
{
	qsrand(time(0));
	KAboutData about = Palapeli::aboutData();
	KCmdLineArgs::init(argc, argv, &about);

	KCmdLineOptions options;
	options.add("i").add("import URL", ki18n("Puzzle archive to import into the library"));
	options.add("n").add("nogui", ki18n("Do not open main window (to be used with --import)"));
	options.add("+[URL]", ki18n("Puzzle archive to open and play"));
	KCmdLineArgs::addCmdLineOptions(options);

	KApplication app;
	app.setWindowIcon(KIcon("preferences-plugin"));
	KGlobal::locale()->insertCatalog("libkdegames");

	//The GUI (and therefore the event loop) is only needed when ppMgr()->init() returns true.
	return ppMgr()->init() ? app.exec() : 0;
}

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

#include <ctime>
#include <QLabel>
#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>
#include <KGlobal>
#include <KLocale>

int main(int argc, char** argv)
{
	qsrand(time(0));
	KAboutData about("palapeli", 0, ki18nc("The application's name", "Palapeli"), "1.0", ki18n("KDE Jigsaw Puzzle Game"), KAboutData::License_GPL, ki18n("Copyright 2009, Stefan Majewsky"));
	about.addAuthor(ki18n("Stefan Majewsky"), KLocalizedString(), "majewsky@gmx.net", "http://majewsky.wordpress.com");
	KCmdLineArgs::init(argc, argv, &about);

	KApplication app;
	KGlobal::locale()->insertCatalog("libkdegames");

	QLabel label("Temporary placeholder widget"); label.show();
	return app.exec();
}
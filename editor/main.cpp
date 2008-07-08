/***************************************************************************
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

#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>
#include <KIcon>
#include <KLocalizedString>

int main(int argc, char** argv)
{
	KAboutData about("paladesign", "paladesign", ki18nc("The application's name", "Paladesign"), "0.1", ki18n("Editor for Palapeli patterns"), KAboutData::License_GPL, ki18n("(c) 2008, the Palapeli team"));
	about.addAuthor(ki18n("Stefan Majewsky"), KLocalizedString(), "majewsky@gmx.net");
	KCmdLineArgs::init(argc, argv, &about);

	KApplication app;
	app.setWindowIcon(KIcon("preferences-plugin"));

	Paladesign::Manager manager;
	manager.window()->show();
	return app.exec();
}

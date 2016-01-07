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


#include <QApplication>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <QCommandLineOption>

int main(int argc, char** argv)
{
	qsrand(time(0));
	KAboutData about("palapeli", i18nc("The application's name", "Palapeli"), "2.0", i18n("KDE Jigsaw Puzzle Game"), KAboutLicense::GPL, i18n("Copyright 2009, 2010, Stefan Majewsky"));
	about.addAuthor(i18n("Stefan Majewsky"), QString(), "majewsky@gmx.net", "http://majewsky.wordpress.com");
	about.addCredit (i18n ("Johannes Loehnert"),
			 i18n ("The option to preview the completed puzzle"),
			 "loehnert.kde@gmx.de");
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("palapeli");

    QCommandLineParser parser;
    KAboutData::setApplicationData(about);
    parser.addVersionOption();
    parser.addHelpOption();
        parser.addPositionalArgument(QStringLiteral("puzzlefile"), i18n("Path to puzzle file (will be opened if -i is not given)"));
        parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("i") << QStringLiteral("import"), i18n("Import the given puzzle file into the local collection (does nothing if no puzzle file is given)")));
        parser.addOption(QCommandLineOption(QStringList() << QLatin1String(""), i18n("If the -i/--import option is specified, the main window will not be shown after importing the given puzzle.")));

    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);


	//NOTE: Syntax errors are reported on stderr, while file errors are presented to the user.
	if (parser.isSet("import"))
		//perform import request
        new Palapeli::ImportHelper(parser.value("import"));
    else {
        const QStringList args = parser.positionalArguments();
        QString path;
        if (args.count()>1) {
            path = args.at(1);
        }
		//no import request, show main window
        (new Palapeli::MainWindow(path))->show();
    }
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("palapeli")));
	return app.exec();
}

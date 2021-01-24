/*
    SPDX-FileCopyrightText: 2009, 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "importhelper.h"
#include "window/mainwindow.h"
#include "palapeli_version.h"

#include <ctime>
#include <KAboutData>
#include <KCrash>

#include <QApplication>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <QCommandLineOption>

int main(int argc, char** argv)
{
    // Fixes blurry icons with fractional scaling
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("palapeli");

    KAboutData about(QStringLiteral("palapeli"),
                     i18nc("The application's name", "Palapeli"),
                     QStringLiteral(PALAPELI_VERSION_STRING),
                     i18n("Jigsaw Puzzle Game"),
                     KAboutLicense::GPL,
                     i18n("Copyright 2009, 2010, Stefan Majewsky"));
    about.addAuthor(i18n("Stefan Majewsky"), QString(), QStringLiteral("majewsky@gmx.net"), QStringLiteral("https://majewsky.wordpress.com/"));
    about.addCredit (i18n ("Johannes Loehnert"),
            i18n ("The option to preview the completed puzzle"),
            QStringLiteral("loehnert.kde@gmx.de"));
    QCommandLineParser parser;
    KAboutData::setApplicationData(about);
    KCrash::initialize();
        parser.addPositionalArgument(QStringLiteral("puzzlefile"), i18n("Path to puzzle file (will be opened if -i is not given)"));
        parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("i") << QStringLiteral("import"), i18n("Import the given puzzle file into the local collection (does nothing if no puzzle file is given). The main window will not be shown after importing the given puzzle.")));

    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);


    //NOTE: Syntax errors are reported on stderr, while file errors are presented to the user.
    if (parser.isSet(QStringLiteral("import")))
        //perform import request
        new Palapeli::ImportHelper(parser.value(QStringLiteral("import")));
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

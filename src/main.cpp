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
#include <KDBusService>
#define HAVE_KICONTHEME __has_include(<KIconTheme>)
#if HAVE_KICONTHEME
#include <KIconTheme>
#endif

#define HAVE_STYLE_MANAGER __has_include(<KStyleManager>)
#if HAVE_STYLE_MANAGER
#include <KStyleManager>
#endif
#include <QApplication>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include <iostream>

int main(int argc, char** argv)
{
#if HAVE_KICONTHEME
    KIconTheme::initTheme();
#endif
    QApplication app(argc, argv);
#if HAVE_STYLE_MANAGER
    KStyleManager::initStyle();
#else // !HAVE_STYLE_MANAGER
#if defined(Q_OS_MACOS) || defined(Q_OS_WIN)
    QApplication::setStyle(QStringLiteral("breeze"));
#endif // defined(Q_OS_MACOS) || defined(Q_OS_WIN)
#endif // HAVE_STYLE_MANAGER
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("palapeli"));

    KAboutData about(QStringLiteral("palapeli"),
                     i18nc("The application's name", "Palapeli"),
                     QStringLiteral(PALAPELI_VERSION_STRING),
                     i18n("Jigsaw Puzzle Game"),
                     KAboutLicense::GPL,
                     i18n("Copyright 2009, 2010, Stefan Majewsky"),
                     QString(),
                     QStringLiteral("https://apps.kde.org/palapeli"));
    about.addAuthor(i18n("Stefan Majewsky"), QString(), QStringLiteral("majewsky@gmx.net"), QStringLiteral("https://majewsky.wordpress.com/"));
    about.addCredit (i18n ("Johannes Loehnert"),
            i18n ("The option to preview the completed puzzle"),
            QStringLiteral("loehnert.kde@gmx.de"));

    KAboutData::setApplicationData(about);
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("palapeli")));

    KCrash::initialize();

    QCommandLineParser parser;
    parser.addPositionalArgument(QStringLiteral("puzzlefile"), i18n("Path to puzzle file (will be opened if -i is not given)"));
    const QCommandLineOption importOption(QStringList() << QStringLiteral("i") << QStringLiteral("import"), i18n("Import the given puzzle file into the local collection (does nothing if no puzzle file is given). The main window will not be shown after importing the given puzzle."));
    parser.addOption(importOption);
    about.setupCommandLine(&parser);

    parser.process(app);
    about.processCommandLine(&parser);

    KDBusService service;

    //NOTE: Syntax errors are reported on stderr, while file errors are presented to the user.
    if (parser.isSet(importOption)) {
        //perform import request

        if (parser.positionalArguments().isEmpty()) {
            std::cout << i18n("No file to import given").toStdString() << std::endl;
            return 1;
        }

        new Palapeli::ImportHelper(parser.positionalArguments().first());
    } else {
        const QStringList args = parser.positionalArguments();
        QString path;
        if (!args.isEmpty()) {
            path = args.first();
        }
        //no import request, show main window
        (new Palapeli::MainWindow(path))->show();
    }

    return app.exec();
}

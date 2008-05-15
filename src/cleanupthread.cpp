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

#include "cleanupthread.h"
#include "strings.h"

#include <QDir>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <KConfig>
#include <KConfigGroup>
#include <KGlobal>
#include <KStandardDirs>

namespace Palapeli
{
	
	QMutex cleanupThreadMutex;

	struct CleanupSavegameData
	{
		CleanupSavegameData(const QString& gameName) : name(gameName), valid(true) {}
		QString name;
		QString configFile;
		QString imageFile;
		bool valid;
	};

}

QStringList Palapeli::cleanSavegameDirectory(const QStringList& savegames)
{
	QMutexLocker cleanupThreadMutexLocker(&cleanupThreadMutex);
	QList<Palapeli::CleanupSavegameData> games;
	KStandardDirs* ksd = KGlobal::dirs();
	//gather information about the savegames
	foreach (QString gameName, savegames)
	{
		Palapeli::CleanupSavegameData game(gameName);
		//find configuration file
		QString configFile = Palapeli::Strings::configPath(gameName);
		if (QFile(configFile).exists())
			game.configFile = configFile;
		else
			game.valid = false;
		//find image file
		//ATTENTION: This KConfig should not do any changes to the configuration file.
		KConfig config(configFile);
		KConfigGroup generalGroup(&config, Palapeli::Strings::GeneralGroupKey);
		QString imageFile = Palapeli::Strings::dataPath(generalGroup.readEntry(Palapeli::Strings::ImageFileKey, QString()));
		if (QFile(imageFile).exists())
			game.imageFile = imageFile;
		else
			game.valid = false;
		//finished
		games << game;
	}
	//compile a list of the files in all local appdata directories
	QStringList files;
	QDir dir;
	foreach (QString dirName, ksd->findDirs("appdata", Palapeli::Strings::SavegameDir))
	{
		dir.setPath(dirName);
		foreach (QFileInfo file, dir.entryInfoList(QDir::Files))
			files << file.absoluteFilePath();
	}
	//remove the files associated with games from this list
	foreach (Palapeli::CleanupSavegameData game, games)
	{
		if (game.valid)
		{
			files.removeAll(game.configFile);
			files.removeAll(game.imageFile);
		}
	}
	//delete the files
	foreach (QString file, files)
		QFile(file).remove();
	//compile a list of invalid games
	QStringList invalidGames;
	foreach (Palapeli::CleanupSavegameData game, games)
	{
		if (!game.valid)
			invalidGames << game.name;
	}
	return invalidGames;
}

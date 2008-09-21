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

#include "puzzleinfo.h"
#include "library.h"

#include <QMutex>
#include <KConfig>
#include <KConfigGroup>
#include <KDesktopFile>

//BEGIN Palapeli::PuzzleInfo

Palapeli::PuzzleInfo::PuzzleInfo(const QString& theIdentifier)
	: identifier(theIdentifier)
	, mutex(new QMutex)
{
	//The other data is provided by the PuzzleInfoLoader.
}

Palapeli::PuzzleInfo::~PuzzleInfo()
{
	delete mutex;
}

//END Palapeli::PuzzleInfo

//BEGIN Palapeli::PuzzleInfoLoader

Palapeli::PuzzleInfoLoader::PuzzleInfoLoader(Palapeli::PuzzleInfo* puzzleInfo)
	: m_puzzleInfo(puzzleInfo)
{
}

Palapeli::PuzzleInfo* Palapeli::PuzzleInfoLoader::puzzleInfo() const
{
	return m_puzzleInfo;
}

void Palapeli::PuzzleInfoLoader::run()
{
	m_puzzleInfo->mutex->lock();
	//load configuration
	KDesktopFile* mainConfig = new KDesktopFile(Palapeli::Library::findFile(m_puzzleInfo->identifier, Palapeli::Library::MainConfigFile));
	//read basic things from configuration
	m_puzzleInfo->name = mainConfig->readName();
	m_puzzleInfo->comment = mainConfig->readComment();
	m_puzzleInfo->author = mainConfig->desktopGroup().readEntry("X-KDE-PluginInfo-Author", QString());
	m_puzzleInfo->pieceCount = KConfigGroup(mainConfig, "Palapeli").readEntry("PieceCount", 0);
	//get images from configuration
	const QString imageName = mainConfig->readIcon().remove('/'); //slashes are removed to avoid directory changing
	const QString imageFile = Palapeli::Library::findFile(imageName, Palapeli::Library::ImageFile);
	const int thumbnailSize = Palapeli::Library::IconSize;
	m_puzzleInfo->image.load(imageFile);
	m_puzzleInfo->thumbnail = m_puzzleInfo->image.scaled(thumbnailSize, thumbnailSize, Qt::KeepAspectRatio);
	//everything loaded - unlock the PuzzleInfo object
	m_puzzleInfo->mutex->unlock();
	setFinished(true);
}

//END Palapeli::PuzzleInfoLoader

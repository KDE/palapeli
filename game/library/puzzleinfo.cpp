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
#include "librarybase.h"

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

Palapeli::PuzzleInfo& Palapeli::PuzzleInfo::operator=(Palapeli::PuzzleInfo& other)
{
	mutex->lock();
	other.mutex->lock();
	identifier = other.identifier;
	name = other.name;
	comment = other.comment;
	author = other.author;
	image = other.image;
	thumbnail = other.thumbnail;
	pieceCount = other.pieceCount;
	other.mutex->unlock();
	mutex->unlock();
	return *this;
}

Palapeli::PuzzleInfo::~PuzzleInfo()
{
	delete mutex;
}

//END Palapeli::PuzzleInfo

//BEGIN Palapeli::PuzzleInfoLoader

Palapeli::PuzzleInfoLoader::PuzzleInfoLoader(Palapeli::PuzzleInfo* puzzleInfo, Palapeli::LibraryBase* base)
	: m_base(base)
	, m_puzzleInfo(puzzleInfo)
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
	KDesktopFile* mainConfig = new KDesktopFile(m_base->findFile(m_puzzleInfo->identifier, Palapeli::LibraryBase::MainConfigFile));
	//read basic things from configuration
	m_puzzleInfo->name = mainConfig->readName();
	m_puzzleInfo->comment = mainConfig->readComment();
	m_puzzleInfo->author = mainConfig->desktopGroup().readEntry("X-KDE-PluginInfo-Author", QString());
	m_puzzleInfo->pieceCount = KConfigGroup(mainConfig, "X-Palapeli").readEntry("PieceCount", 0);
	//get images from configuration
	m_puzzleInfo->imageFile = mainConfig->readIcon().remove('/'); //slashes are removed to avoid directory changing
	const QString imageFileName = m_base->findFile(m_puzzleInfo->imageFile, Palapeli::LibraryBase::ImageFile);
	const int thumbnailSize = Palapeli::Library::IconSize;
	m_puzzleInfo->image.load(imageFileName);
	m_puzzleInfo->thumbnail = m_puzzleInfo->image.scaled(thumbnailSize, thumbnailSize, Qt::KeepAspectRatio);
	//everything loaded - unlock the PuzzleInfo object
	m_puzzleInfo->mutex->unlock();
	setFinished(true);
}

//END Palapeli::PuzzleInfoLoader

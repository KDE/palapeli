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

#include "librarybase.h"
#include "library.h"
#include "puzzleinfo.h"

#include <QFileInfo>
#include <KConfig>
#include <KConfigGroup>
#include <KIO/NetAccess>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardDirs>
#include <KTar>
#include <KTempDir>
#include <KTemporaryFile>

const QString StandardFolder("palapeli/puzzlelibrary/");
const QString MainFileName("%1.desktop");
const QString StateFileName("%1.cfg");
const QString ImageFileName("%1");

const QString ArchiveConfigFileName("palapeli-archive.cfg");
const QString ArchiveConfigGroupName("Palapeli Archive");
const QString ArchiveConfigIdentifierKey("Identifier");

//BEGIN Palapeli::LibraryStandardBase

Palapeli::LibraryStandardBase::LibraryStandardBase()
	: m_dirs(new KStandardDirs)
{
}

Palapeli::LibraryStandardBase::~LibraryStandardBase()
{
	delete m_dirs;
}

QString Palapeli::LibraryStandardBase::findFile(const QString& identifier, Palapeli::LibraryBase::FileType type, bool onlyLocal) const
{
	switch (type)
	{
		case Palapeli::LibraryBase::MainConfigFile:
			if (onlyLocal)
				return m_dirs->locateLocal("data", StandardFolder + MainFileName.arg(identifier));
			else
				return m_dirs->locate("data", StandardFolder + MainFileName.arg(identifier));
			break;
		case Palapeli::LibraryBase::StateConfigFile:
			return m_dirs->locateLocal("data", StandardFolder + StateFileName.arg(identifier));
			break;
		case Palapeli::LibraryBase::ImageFile:
			if (onlyLocal)
				return m_dirs->locateLocal("data", StandardFolder + ImageFileName.arg(identifier));
			else
				return m_dirs->locate("data", StandardFolder + ImageFileName.arg(identifier));
			break;
		default: //will never be reached, but g++ does not like it without a default
			return QString();
	}
}

QStringList Palapeli::LibraryStandardBase::findEntries() const
{
	QStringList identifiers;
	const QStringList files = m_dirs->findAllResources("data", StandardFolder + MainFileName.arg("*"), KStandardDirs::NoDuplicates);
	foreach (const QString& mainConfigPath, files)
		identifiers << mainConfigPath.section('/', -1, -1).section('.', 0, 0);
	return identifiers;
}

//END Palapeli::LibraryStandardBase

//BEGIN Palapeli::LibraryArchiveBase

Palapeli::LibraryArchiveBase::LibraryArchiveBase(const KUrl& url)
	: m_url(url)
	, m_cache(0)
{
	//make archive available locally
	QString archiveFile;
	if (!url.isLocalFile())
	{
		if (!KIO::NetAccess::download(url, archiveFile, 0))
			return;
	}
	else
		archiveFile = url.path();
	//open archive and extract into temporary directory
	KTar tar(archiveFile, "application/x-bzip");
	if (!tar.open(QIODevice::ReadOnly))
	{
		KIO::NetAccess::removeTempFile(archiveFile);
		return;
	}
	const KArchiveDirectory* archiveDir = tar.directory();
	m_cache = new KTempDir;
	const QString cachePath = m_cache->name(); //note: includes trailing slash
	archiveDir->copyTo(cachePath);
	tar.close();
	//read identifier from archive config
	KConfig config(cachePath + ArchiveConfigFileName);
	KConfigGroup configGroup(&config, ArchiveConfigGroupName);
	m_identifier = configGroup.readEntry(ArchiveConfigIdentifierKey, QString());
	if (m_identifier.isNull()) //failed to read archive configuration
	{
		m_cache->unlink();
		delete m_cache;
		KIO::NetAccess::removeTempFile(archiveFile);
		return;
	}
	//cleanup
	KIO::NetAccess::removeTempFile(archiveFile);
}

Palapeli::LibraryArchiveBase::~LibraryArchiveBase()
{
	if (m_cache)
	{
		m_cache->unlink();
		delete m_cache;
	}
}

QString Palapeli::LibraryArchiveBase::findFile(const QString& identifier, Palapeli::LibraryBase::FileType type, bool onlyLocal) const
{
	const QString directory = m_cache->name();
	//for images, the identifier check below has to be overriden because the image's name may be different
	if (type == Palapeli::LibraryBase::ImageFile)
	{
		if (onlyLocal)
			return QString(); //no local storage available for this type
		else
			return directory + ImageFileName.arg(identifier);
	}
	//check identifier (has to be the same as the game identifier)
	const QString identString = m_identifier.toString();
	if (identifier != identString)
		return QString();
	switch (type)
	{
		case Palapeli::LibraryBase::MainConfigFile:
			if (onlyLocal)
				return QString(); //no local storage available for this type
			else
				return directory + MainFileName.arg(identString);
			break;
		case Palapeli::LibraryBase::StateConfigFile:
			//for state config, use normal config folder instead
			return KStandardDirs::locateLocal("data", StandardFolder + StateFileName.arg(identifier));
			break;
		default: //will never be reached, but g++ does not like it without a default
			return QString();
	}
}

QStringList Palapeli::LibraryArchiveBase::findEntries() const
{
	return QStringList() << m_identifier.toString();
}

bool Palapeli::LibraryArchiveBase::create(Palapeli::Library* sourceLibrary, const QString& identifier)
{
	//collect information about the puzzle to be inserted
	const QString mainConfigSourcePath = sourceLibrary->base()->findFile(identifier, Palapeli::LibraryBase::MainConfigFile);
	Palapeli::PuzzleInfo* info = sourceLibrary->infoForPuzzle(identifier);
	if (!info)
		return false;
	const QString imageSourcePath = sourceLibrary->base()->findFile(info->imageFile, Palapeli::LibraryBase::ImageFile);
	if (mainConfigSourcePath.isEmpty() || imageSourcePath.isEmpty())
		return false;
	QFile mainConfigSource(mainConfigSourcePath);
	QFile imageSource(imageSourcePath);
	if (!mainConfigSource.exists() || !imageSource.exists())
		return false;
	//flush the contents of this archive before inserting the new puzzle
	if (m_cache)
	{
		m_cache->unlink();
		delete m_cache;
	}
	m_cache = new KTempDir;
	const QString cachePath = m_cache->name();
	//create a new unique identifier for this puzzle
	m_identifier = QUuid::createUuid();
	const QString identString = m_identifier.toString();
	//insert the new puzzle
	const QString mainConfigTargetPath = cachePath + MainFileName.arg(identString);
	const QString imageTargetPath = cachePath + QFileInfo(imageSourcePath).fileName(); //the name has to remain the same because it's referenced in the main config file
	if (!mainConfigSource.copy(mainConfigTargetPath))
		return false;
	if (!imageSource.copy(imageTargetPath))
		return false;
	//write archive configuration
	KConfig config(cachePath + ArchiveConfigFileName);
	KConfigGroup configGroup(&config, ArchiveConfigGroupName);
	configGroup.writeEntry(ArchiveConfigIdentifierKey, identString);
	config.sync(); //TODO: does this work correctly?
	//compress archive to temporary file
	KTemporaryFile tempFile;
	tempFile.setSuffix(".pala");
	if (!tempFile.open())
	{
		tempFile.remove();
		return false;
	}
	KTar tar(tempFile.fileName(), "application/x-bzip");
	if (!tar.open(QIODevice::WriteOnly))
	{
		tempFile.remove();
		return false;
	}
	if (!tar.addLocalDirectory(cachePath, QLatin1String(".")))
	{
		tempFile.remove();
		return false;
	}
	if (!tar.close())
	{
		tempFile.remove();
		return false;
	}
	//upload to archive location
	if (!KIO::NetAccess::upload(tempFile.fileName(), m_url, 0))
		KMessageBox::error(0, KIO::NetAccess::lastErrorString());
	//cleanup
	tempFile.remove();
	return true;
}

//END Palapeli::LibraryArchiveBase

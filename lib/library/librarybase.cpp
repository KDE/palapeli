/***************************************************************************
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

const QString ArchiveConfigFileName("palapeli-archive.cfg");
const QString ArchiveConfigGroupName("Palapeli Archive");
const QString ArchiveConfigIdentifierKey("Identifier");

//BEGIN Palapeli::LibraryBase

bool Palapeli::LibraryBase::insertEntry(const QString& identifier, Palapeli::Library* sourceLibrary)
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
	//insert the new puzzle (note: the image name has to remain the same because it's referenced in the main config file)
	const QString mainConfigTargetPath = findFile(identifier, Palapeli::LibraryBase::MainConfigFile, true);
	const QString imageTargetPath = findFile(QFileInfo(imageSourcePath).fileName(), Palapeli::LibraryBase::ImageFile, true);
	if (!QFileInfo(mainConfigTargetPath).exists() && !mainConfigSource.copy(mainConfigTargetPath))
		return false;
	if (!QFileInfo(imageTargetPath).exists() && !imageSource.copy(imageTargetPath))
		return false;
	emit entryInserted(identifier);
	return true;
}

bool Palapeli::LibraryBase::insertEntries(Palapeli::Library* sourceLibrary)
{
	const QStringList entries = sourceLibrary->base()->findEntries();
	foreach (const QString& identifier, entries)
	{
		if (!insertEntry(identifier, sourceLibrary))
			return false;
	}
	return true;
}

bool Palapeli::LibraryBase::canRemoveEntry(const QString& identifier, Palapeli::Library* library)
{
	//security check
	if (library->base() != this)
		return false;
	//check if main config is stored in a read-only place
	QString path1 = findFile(identifier, Palapeli::LibraryBase::MainConfigFile, false);
	QString path2 = findFile(identifier, Palapeli::LibraryBase::MainConfigFile, true);
	if (path1 != path2)
		return false;
	//like above for the image
	Palapeli::PuzzleInfo* info = library->infoForPuzzle(identifier);
	path1 = findFile(info->imageFile, Palapeli::LibraryBase::ImageFile, false);
	path2 = findFile(info->imageFile, Palapeli::LibraryBase::ImageFile, true);
	if (path1 != path2)
		return false;
	//no bad indications at this point (at least in this generic implementation)
	return true;
}

bool Palapeli::LibraryBase::removeEntry(const QString& identifier, Palapeli::Library* library)
{
	//security check
	if (library->base() != this)
		return false;
	//find files
	const QString mainConfigPath = findFile(identifier, Palapeli::LibraryBase::MainConfigFile, true);
	Palapeli::PuzzleInfo* info = library->infoForPuzzle(identifier);
	if (!info)
		return false;
	const QString imagePath = findFile(info->imageFile, Palapeli::LibraryBase::ImageFile, true);
	if (mainConfigPath.isEmpty() || imagePath.isEmpty())
		return false;
	QFile mainConfig(mainConfigPath);
	QFile image(imagePath);
	//check if image is used by another entry
	QStringList entries = findEntries();
	bool deleteImage = true;
	foreach (const QString& otherIdentifier, entries)
	{
		if (otherIdentifier == identifier) //the obvious case of other image == own image
			continue;
		Palapeli::PuzzleInfo* otherInfo = library->infoForPuzzle(otherIdentifier);
		if (otherInfo->imageFile == info->imageFile)
			deleteImage = false;
	}
	//perform deletion
	if (mainConfig.exists())
		mainConfig.remove();
	if (image.exists() && deleteImage)
		image.remove();
	emit entryRemoved(identifier);
	return true;
}

void Palapeli::LibraryBase::reportNewEntry(const QString& identifier)
{
	//ensure existence of this entry
	if (!findFile(identifier, Palapeli::LibraryBase::MainConfigFile).isEmpty())
		emit entryInserted(identifier);
}

//END Palapeli::LibraryBase

//BEGIN Palapeli::LibraryStandardBase

Palapeli::LibraryStandardBase* Palapeli::LibraryStandardBase::self()
{
	static Palapeli::LibraryStandardBase* theOneAndOnly = new Palapeli::LibraryStandardBase;
	return theOneAndOnly;
}

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
				return m_dirs->locateLocal("data", StandardFolder + identifier);
			else
				return m_dirs->locate("data", StandardFolder + identifier);
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

Palapeli::Library* Palapeli::standardLibrary()
{
	static Palapeli::Library theOneAndOnly(Palapeli::LibraryStandardBase::self());
	return &theOneAndOnly;
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
	Q_UNUSED(onlyLocal)
	const QString directory = m_cache->name();
	//For images, use the given identifier (the image's name may be different from the puzzle's identifier). For config files, always use the standard identifier.
	switch (type)
	{
		case Palapeli::LibraryBase::MainConfigFile:
			return directory + MainFileName.arg(m_identifier.toString());
			break;
		case Palapeli::LibraryBase::StateConfigFile:
			//for state config, use normal config folder instead
			return KStandardDirs::locateLocal("data", StandardFolder + StateFileName.arg(m_identifier.toString()));
			break;
		case Palapeli::LibraryBase::ImageFile:
			return directory + identifier;
		default: //will never be reached, but g++ does not like it without a default
			return QString();
	}
}

QStringList Palapeli::LibraryArchiveBase::findEntries() const
{
	return m_identifier.isNull() ? QStringList() : (QStringList() << m_identifier.toString());
}

bool Palapeli::LibraryArchiveBase::insertEntry(const QString& identifier, Palapeli::Library* sourceLibrary)
{
	//save the old archive
	KTempDir* oldCache = m_cache;
	const QUuid oldIdentifier(m_identifier);
	//create a new archive
	m_cache = new KTempDir;
	m_identifier = QUuid::createUuid();
	const QString cachePath = m_cache->name();
	const QString identString = m_identifier.toString();
	//try to copy the files
	if (Palapeli::LibraryBase::insertEntry(identifier, sourceLibrary))
	{
		//operation succeeded - discard old cache
		if (oldCache)
		{
			oldCache->unlink();
			delete oldCache;
		}
		if (!oldIdentifier.isNull())
			emit entryRemoved(oldIdentifier.toString());
	}
	else
	{
		//operation failed - restore old cache
		m_cache->unlink();
		delete m_cache;
		m_cache = oldCache;
		m_identifier = oldIdentifier;
		return false;
	}
	//write archive configuration
	KConfig config(cachePath + ArchiveConfigFileName);
	KConfigGroup configGroup(&config, ArchiveConfigGroupName);
	configGroup.writeEntry(ArchiveConfigIdentifierKey, identString);
	config.sync();
	//compress archive to temporary file
	bool success = true;
	KTemporaryFile tempFile;
	tempFile.setSuffix(".pala");
	if (!tempFile.open())
		success = false;
	else
	{
		KTar tar(tempFile.fileName(), "application/x-bzip");
		if (!tar.open(QIODevice::WriteOnly))
			success = false;
		else if (!tar.addLocalDirectory(cachePath, QLatin1String(".")))
			success = false;
		else if (!tar.close())
			success = false;
		//upload to archive location
		else if (!KIO::NetAccess::upload(tempFile.fileName(), m_url, 0))
		{
			KMessageBox::error(0, KIO::NetAccess::lastErrorString());
			success = false;
		}
	}
	return success;
}

//END Palapeli::LibraryArchiveBase

#include "librarybase.moc"

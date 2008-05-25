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

#include "gamestorage.h"
#include "gamestorageattribs.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <KConfig>
#include <KConfigGroup>
#include <KDebug> //used for kError (just a hint for me as I usually delete KDebug headers before commiting)
#include <KIO/NetAccess>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardDirs>
#include <KTar>
#include <KTempDir>
#include <KTemporaryFile>

namespace Palapeli
{

	class GameStoragePrivate
	{
		public:
			GameStoragePrivate(const QString& baseDirectory);
			~GameStoragePrivate();

			//configuration access and "meta data"
			QString m_baseDirectory;
			QString m_filePathTemplate;
			KConfig *m_config;
			bool m_accessible; //false when the storage directory can not be created or is not writeable
			int m_nextItemNumber; //index which can be used when the next item is added to the storage
			int m_nextDepNumber; //same for dependencies
			//configuration groups and storage for item properties
			KConfigGroup *m_idGroup;
			QHash<QUuid, int> m_idHash; //structure: id -> item number
			KConfigGroup *m_extGroup;
			QHash<int, QString> m_extHash; //structure: item number -> extension
			KConfigGroup *m_typeGroup;
			QHash<int, int> m_typeHash; //structure: item number -> type
			KConfigGroup *m_metaGroup;
			QHash<int, QString> m_metaHash; //structure: item number -> meta data string
			KConfigGroup *m_depGroup;
			QList<int> m_depNumbers;
			QHash<int, QPair<int, int> > m_depHash; //structure: dependency number -> (source item number, target item number)
	};

}

const QString configPath("%1/gamestoragerc");
const QString idGroupKey("Identifiers");
const QString extGroupKey("Extensions");
const QString typeGroupKey("Types");
const QString metaGroupKey("Metadata");
const QString depGroupKey("Dependencies");
const QString entryKey("%1");
const QString depListKey("DependencyKeys");
const QString depSourceKey("Source-%1");
const QString depTargetKey("Target-%1");

inline uint qHash(const QUuid& id)
{
	return qHash(id.toString());
}

#include <KDebug>

Palapeli::GameStoragePrivate::GameStoragePrivate(const QString& baseDirectory)
	: m_baseDirectory(baseDirectory)
	, m_filePathTemplate(baseDirectory + "/%1%2")
	, m_config(0)
	, m_accessible(true)
	, m_nextItemNumber(0)
	, m_nextDepNumber(0)
	, m_idGroup(0)
	, m_extGroup(0)
	, m_typeGroup(0)
	, m_metaGroup(0)
{
	//check availability of the base directory
	QFileInfo fi(baseDirectory);
	bool isDir = fi.isDir(), exists = fi.exists();
	if (!isDir && exists)
	{
		kError() << "Could not create game storage at: " << baseDirectory;
		KMessageBox::error(0, i18n("Game storage could not be created. Changes won't be saved."));
		m_accessible = false;
		return;
	}
	if (!exists)
	{
		if (!QDir().mkdir(baseDirectory))
		{
			kError() << "Could not create game storage at: " << baseDirectory;
			KMessageBox::error(0, i18n("Game storage could not be created. Changes won't be saved."));
			m_accessible = false;
			return;
		}
	}
	//open configuration file
	m_config = new KConfig(configPath.arg(baseDirectory));
	m_typeGroup = new KConfigGroup(m_config, typeGroupKey);
	//read IDs
	m_idGroup = new KConfigGroup(m_config, idGroupKey);
	QMap<QString,QString> idMap = m_idGroup->entryMap();
	QMapIterator<QString,QString> iterIdMap(idMap);
	while (iterIdMap.hasNext())
	{
		iterIdMap.next();
		m_idHash[QUuid(iterIdMap.value())] = iterIdMap.key().toInt();
		m_nextItemNumber = qMax(m_nextItemNumber, iterIdMap.key().toInt()); //see the end of this function for details
	}
	//read extensions
	m_extGroup = new KConfigGroup(m_config, extGroupKey);
	QMap<QString,QString> extMap = m_extGroup->entryMap();
	QMapIterator<QString,QString> iterExtMap(extMap);
	while (iterExtMap.hasNext())
	{
		iterExtMap.next();
		m_extHash[iterExtMap.key().toInt()] = iterExtMap.value();
	}
	//read types
	m_typeGroup = new KConfigGroup(m_config, typeGroupKey);
	QMap<QString,QString> typeMap = m_typeGroup->entryMap();
	QMapIterator<QString,QString> iterTypeMap(typeMap);
	while (iterTypeMap.hasNext())
	{
		iterTypeMap.next();
		m_typeHash[iterTypeMap.key().toInt()] = iterTypeMap.value().toInt();
	}
	//read meta data
	m_metaGroup = new KConfigGroup(m_config, metaGroupKey);
	QMap<QString,QString> metaMap = m_metaGroup->entryMap();
	QMapIterator<QString,QString> iterMetaMap(metaMap);
	while (iterMetaMap.hasNext())
	{
		iterMetaMap.next();
		m_metaHash[iterMetaMap.key().toInt()] = iterMetaMap.value();
	}
	//read dependencies
	m_depGroup = new KConfigGroup(m_config, depGroupKey);
	m_depNumbers = m_depGroup->readEntry(depListKey, QList<int>());
	foreach (int depNumber, m_depNumbers)
	{
		m_nextDepNumber = qMax(m_nextDepNumber, depNumber); //see the end of this function for details
		m_depHash[depNumber] = QPair<int,int>(
			m_depGroup->readEntry(depSourceKey.arg(depNumber), 0),
			m_depGroup->readEntry(depTargetKey.arg(depNumber), 0)
		);
	}
	//next free item number is the highest item number found in the configuration values plus one
	++m_nextItemNumber;
	++m_nextDepNumber;
}

Palapeli::GameStoragePrivate::~GameStoragePrivate()
{
	delete m_config;
	delete m_idGroup;
	delete m_extGroup;
	delete m_typeGroup;
	delete m_metaGroup;
	delete m_depGroup;
}

Palapeli::GameStorage::GameStorage()
	: d(new Palapeli::GameStoragePrivate(KStandardDirs::locateLocal("appdata", "gamestorage")))
{
}

Palapeli::GameStorage::GameStorage(const QString& baseDirectory)
	: d(new Palapeli::GameStoragePrivate(baseDirectory))
{
}

Palapeli::GameStorage::~GameStorage()
{
	delete d;
}

bool Palapeli::GameStorage::accessible() const
{
	return d->m_accessible;
}

QString Palapeli::GameStorage::baseDirectory() const
{
	return d->m_baseDirectory;
}

Palapeli::GameStorageItem Palapeli::GameStorage::addItem(const QString& rawExtension, int type)
{
	if (!d->m_accessible)
		return Palapeli::GameStorageItem(QUuid(), this);
	//extension has to start with a dot
	QChar theDot('.');
	QString extension = rawExtension;
	if (!extension.startsWith(theDot))
		extension.prepend(theDot);
	//insert item into internal hashes
	QUuid id = QUuid::createUuid();
	d->m_idHash[id] = d->m_nextItemNumber;
	d->m_extHash[d->m_nextItemNumber] = extension;
	d->m_typeHash[d->m_nextItemNumber] = type;
	d->m_metaHash[d->m_nextItemNumber] = QString();
	//insert item into configuration
	QString key = entryKey.arg(d->m_nextItemNumber);
	d->m_idGroup->writeEntry(key, id.toString());
	d->m_extGroup->writeEntry(key, extension);
	d->m_typeGroup->writeEntry(key, type);
	d->m_metaGroup->writeEntry(key, QString());
	d->m_config->sync();
	//iterate item number for next object to be added
	++d->m_nextItemNumber;
	return Palapeli::GameStorageItem(id, this);
}

Palapeli::GameStorageItem Palapeli::GameStorage::addItem(const KUrl& source, int type)
{
	if (!d->m_accessible)
		return Palapeli::GameStorageItem(QUuid(), this);
	//get extension from filename
	static QRegExp extensionExtractor("(\\..*)$");
	QString extension = source.fileName();
	if (extensionExtractor.indexIn(extension, 0) >= 0)
		extension = extensionExtractor.cap(1);
	//generate new unique filename
	QUuid id = QUuid::createUuid();
	QString localFile = d->m_filePathTemplate.arg(id.toString()).arg(extension);
	//download file
	if (source.isLocalFile())
	{
		QFile sourceFile(source.path());
		if (!sourceFile.copy(localFile))
		{
			KMessageBox::error(0, i18n("File could not be copied to the local image storage."));
			return Palapeli::GameStorageItem(); //null identifier
		}
	}
	else
	{
		if (!KIO::NetAccess::download(source, localFile, 0))
		{
			KMessageBox::error(0, KIO::NetAccess::lastErrorString());
			return Palapeli::GameStorageItem(); //null identifier
		}
	}
	//insert item into internal hashes
	d->m_idHash[id] = d->m_nextItemNumber;
	d->m_extHash[d->m_nextItemNumber] = extension;
	d->m_typeHash[d->m_nextItemNumber] = type;
	d->m_metaHash[d->m_nextItemNumber] = QString();
	//insert item into configuration
	QString key = entryKey.arg(d->m_nextItemNumber);
	d->m_idGroup->writeEntry(key, id.toString());
	d->m_extGroup->writeEntry(key, extension);
	d->m_typeGroup->writeEntry(key, type);
	d->m_metaGroup->writeEntry(key, QString());
	d->m_config->sync();
	//iterate item number for next object to be added
	++d->m_nextItemNumber;
	return Palapeli::GameStorageItem(id, this);
}

Palapeli::GameStorageItem Palapeli::GameStorage::item(const QUuid& id)
{
	return Palapeli::GameStorageItem(d->m_accessible && itemExists(id) ? id : QUuid(), this);
}

Palapeli::GameStorageItems Palapeli::GameStorage::queryItems(const Palapeli::GameStorageAttributes& attributes)
{
	Palapeli::GameStorageItems items;
	if (!d->m_accessible)
		return items;
	QHashIterator<QUuid, int> iterIds(d->m_idHash);
	while (iterIds.hasNext())
	{
		Palapeli::GameStorageItem item(iterIds.next().key(), this);
		if (attributes.test(item))
			items << item;
	}
	return items;
}

bool Palapeli::GameStorage::removeItem(const Palapeli::GameStorageItem& item)
{
	QUuid id = item.id();
	if (!d->m_accessible || item.container() != this || id.isNull() || !itemExists(id))
		return false;
	//remove item from file storage
	QFile fi(itemFilePath(id));
	if (fi.exists())
	{
		if (!fi.remove())
			return false;
	}
	//remove item from internal hashes
	int itemNumber = d->m_idHash[id]; //the numerical index of this item
	d->m_idHash.remove(id);
	d->m_extHash.remove(itemNumber);
	d->m_typeHash.remove(itemNumber);
	d->m_metaHash.remove(itemNumber);
	//remove item from configuration
	QString key = entryKey.arg(itemNumber);
	d->m_idGroup->deleteEntry(key);
	d->m_extGroup->deleteEntry(key);
	d->m_typeGroup->deleteEntry(key);
	d->m_metaGroup->deleteEntry(key);
	//remove dependencies
	foreach (int depNumber, d->m_depNumbers)
	{
		QPair<int,int> depContent = d->m_depHash[depNumber];
		if (depContent.first != itemNumber && depContent.second != itemNumber)
			continue;
		//this dependency needs to be removed
		d->m_depHash.remove(depNumber);
		d->m_depNumbers.removeAll(depNumber);
		d->m_depGroup->deleteEntry(depSourceKey.arg(depNumber));
		d->m_depGroup->deleteEntry(depTargetKey.arg(depNumber));
		d->m_depGroup->writeEntry(depListKey, d->m_depNumbers);
	}
	d->m_config->sync();
	return true;
}

bool Palapeli::GameStorage::addDependency(const Palapeli::GameStorageItem& source, const Palapeli::GameStorageItem& target)
{
	if (!d->m_accessible)
		return false;
	if (!itemExists(source) || !itemExists(target))
		return false;
	if (hasDependency(source, target))
		return true; //a dependency does exist - that is what we wanted
	if (source == target)
		return false;
	int sourceItemNumber = d->m_idHash[source.id()];
	int targetItemNumber = d->m_idHash[target.id()];
	//insert dependency into internal storage
	d->m_depNumbers << d->m_nextDepNumber;
	d->m_depHash[d->m_nextDepNumber] = QPair<int,int>(sourceItemNumber, targetItemNumber);
	//insert dependency into configuration
	d->m_depGroup->writeEntry(depSourceKey.arg(d->m_nextDepNumber), sourceItemNumber);
	d->m_depGroup->writeEntry(depTargetKey.arg(d->m_nextDepNumber), targetItemNumber);
	d->m_depGroup->writeEntry(depListKey, d->m_depNumbers);
	d->m_config->sync();
	//iterate dependency number for next object to be added
	++d->m_nextDepNumber;
	return true;
}

bool Palapeli::GameStorage::hasDependency(const Palapeli::GameStorageItem& source, const Palapeli::GameStorageItem& target)
{
	if (!d->m_accessible)
		return false;
	if (!itemExists(source) || !itemExists(target))
		return false;
	if (source == target)
		return false;
	QPair<int, int> wantedDependency(d->m_idHash[source.id()], d->m_idHash[target.id()]);
	return d->m_depHash.key(wantedDependency, 0) != 0; //0 means that no such dependency could be located
}

bool Palapeli::GameStorage::removeDependency(const Palapeli::GameStorageItem& source, const Palapeli::GameStorageItem& target)
{
	if (!d->m_accessible)
		return false;
	if (!itemExists(source) || !itemExists(target))
		return false;
	if (!hasDependency(source, target))
		return true; //a dependency does not exist - that is what we wanted
	if (source == target)
		return false;
	QPair<int, int> dependency(d->m_idHash[source.id()], d->m_idHash[target.id()]);
	int depNumber = d->m_depHash.key(dependency);
	//remove dependency from internal storage
	d->m_depHash.remove(depNumber);
	d->m_depNumbers.removeAll(depNumber);
	//remove dependency from configuration
	d->m_depGroup->deleteEntry(depSourceKey.arg(depNumber));
	d->m_depGroup->deleteEntry(depTargetKey.arg(depNumber));
	d->m_depGroup->writeEntry(depListKey, d->m_depNumbers);
	d->m_config->sync();
	return true;
}

Palapeli::GameStorageItems Palapeli::GameStorage::importItems(GameStorage* storage, const Palapeli::GameStorageItems& items)
{
	if (!d->m_accessible || !storage->d->m_accessible)
		return Palapeli::GameStorageItems();
	Palapeli::GameStorageItems importedItems;
	QHash<int, int> importedItemNumberHash; //needed because the item numbers change during import and the dependencies between items are mapped later; structure: item number in the other storage -> item number in this storage
	QHashIterator<QUuid, int> iterIdHash(storage->d->m_idHash);
	bool restrictedToGivenItemList = !items.empty();
	//import items
	while (iterIdHash.hasNext())
	{
		iterIdHash.next();
		QUuid oldId = iterIdHash.key();
		int oldItemNumber = iterIdHash.value();
		//if restricted to an item list, make sure that this item is included in this list (if not, skip this item)
		if (restrictedToGivenItemList)
		{
			bool found = false;
			foreach (const Palapeli::GameStorageItem& item, items)
			{
				if (item.id() == oldId)
				{
					found = true;
					break;
				}
			}
			if (!found)
				continue;
		}
		//translate ID and item number
		QUuid newId = oldId;
		if (d->m_idHash.contains(oldId))
			newId = QUuid::createUuid();
		int newItemNumber = d->m_nextItemNumber;
		//copy data into this storage
		d->m_idHash[newId] = newItemNumber;
		d->m_extHash[newItemNumber] = storage->d->m_extHash.value(oldItemNumber, QString());
		d->m_typeHash[newItemNumber] = storage->d->m_typeHash.value(oldItemNumber, Palapeli::GameStorageItem::Unspecified);
		d->m_metaHash[newItemNumber] = storage->d->m_metaHash.value(oldItemNumber, QString());
		//save data to configuration of this storage
		QString key = entryKey.arg(newItemNumber);
		d->m_idGroup->writeEntry(key, d->m_idHash.key(newItemNumber).toString());
		d->m_extGroup->writeEntry(key, d->m_extHash[newItemNumber]);
		d->m_typeGroup->writeEntry(key, d->m_typeHash[newItemNumber]);
		d->m_metaGroup->writeEntry(key, d->m_metaHash[newItemNumber]);
		//copy file into this storage
		QFile oldFile(storage->itemFilePath(oldId));
		QString newFilePath = itemFilePath(newId);
		QFile newFile(newFilePath);
		if (newFile.exists())
			newFile.remove();
		if (oldFile.exists())
			oldFile.copy(newFilePath);
		//insert mapping entry
		importedItemNumberHash[oldItemNumber] = newItemNumber;
		++d->m_nextItemNumber;
		importedItems << Palapeli::GameStorageItem(newId, this);
	}
	//import dependencies
	foreach (int oldDepNumber, storage->d->m_depNumbers)
	{
		static const QPair<int, int> defaultValue(0, 0);
		QPair<int, int> oldDep = storage->d->m_depHash.value(oldDepNumber, defaultValue);
		//skip this dependency if items for this dependency do not exist; otherwise map item numbers to new values
		if (!importedItemNumberHash.contains(oldDep.first))
			continue;
		int newSource = importedItemNumberHash.value(oldDep.first);
		if (!importedItemNumberHash.contains(oldDep.second))
			continue;
		int newTarget = importedItemNumberHash.value(oldDep.second);
		//copy data into this storage
		d->m_depNumbers << d->m_nextDepNumber;
		d->m_depHash[d->m_nextDepNumber] = QPair<int, int>(newSource, newTarget);
		//save data to configuration of this storage
		d->m_depGroup->writeEntry(depSourceKey.arg(d->m_nextDepNumber), newSource);
		d->m_depGroup->writeEntry(depTargetKey.arg(d->m_nextDepNumber), newTarget);
		++d->m_nextDepNumber;
	}
	d->m_depGroup->writeEntry(depListKey, d->m_depNumbers);
	d->m_config->sync();
	return importedItems;
}

Palapeli::GameStorageItems Palapeli::GameStorage::importItems(const KUrl& archive)
{
	if (!d->m_accessible)
		return Palapeli::GameStorageItems();
	//download archive
	QString localFile;
	if (archive.isLocalFile())
	{
		if (!KIO::NetAccess::download(archive, localFile, 0))
		{
			KMessageBox::error(0, KIO::NetAccess::lastErrorString());
			return Palapeli::GameStorageItems();
		}
	}
	else
		localFile = archive.path();
	//open archive and extract into local directory
	KTar tar(localFile, "application/x-bzip");
	if (!tar.open(QIODevice::ReadOnly))
		return Palapeli::GameStorageItems();
	const KArchiveDirectory* archiveDir = tar.directory();
	KTempDir tempDir;
	archiveDir->copyTo(tempDir.name());
	//import items from that storage
	Palapeli::GameStorage* gs = new Palapeli::GameStorage(tempDir.name());
	Palapeli::GameStorageItems items = this->importItems(gs);
	delete gs;
	//cleanup
	tempDir.unlink();
	KIO::NetAccess::removeTempFile(localFile);
	return items;
}

bool Palapeli::GameStorage::exportItems(const KUrl& archive, const Palapeli::GameStorageItems& items)
{
	//resolve dependencies until no dependencies are left
	Palapeli::GameStorageItems allItems(items);
	while (true)
	{
		bool addedItem = false;
		//for all items in the list...
		foreach (const Palapeli::GameStorageItem& item, allItems)
		{
			//...look for items which have a dependency on that item, but are not in the list
			QHashIterator<QUuid, int> iterIdHash(d->m_idHash);
			while (iterIdHash.hasNext())
			{
				iterIdHash.next();
				QUuid id = iterIdHash.key();
				Palapeli::GameStorageItem otherItem = this->item(id);
				if (allItems.contains(otherItem)) //item already in list -> uninteresting
					continue;
				if (hasDependency(item, otherItem) || hasDependency(otherItem, item)) //otherItem needs to be inserted
				{
					allItems << otherItem;
					addedItem = true;
				}
			}
		}
		if (!addedItem)
			break;
		//This operation may need to be done more than one time if the inserted items introduce new dependencies.
	}
	//create a game storage in a temp directory
	KTempDir tempDir;
	Palapeli::GameStorage* gs = new Palapeli::GameStorage(tempDir.name());
	gs->importItems(this, allItems);
	delete gs; //really make sure that the configuration has been written
	//save to archive
	KTemporaryFile tempFile;
	tempFile.setSuffix(".psga");
	if (!tempFile.open())
		return false;
	KTar tar(tempFile.fileName(), "application/x-bzip");
	if (!tar.open(QIODevice::WriteOnly))
		return false;
	if (!tar.addLocalDirectory(tempDir.name(), QLatin1String(".")))
		return false;
	if (!tar.close())
		return false;
	//upload archive
	if (!KIO::NetAccess::upload(tempFile.fileName(), archive, 0))
		KMessageBox::error(0, KIO::NetAccess::lastErrorString());
	//cleanup
	tempDir.unlink();
	tempFile.remove();
	return true;
}

bool Palapeli::GameStorage::itemExists(const QUuid& id) const
{
	return d->m_accessible && !id.isNull() && d->m_idHash.contains(id);
}

QString Palapeli::GameStorage::itemExtension(const QUuid& id) const
{
	if (!itemExists(id))
		return QString();
	int itemNumber = d->m_idHash[id]; //the numerical index of this item
	return d->m_extHash.value(itemNumber, QString());
}

int Palapeli::GameStorage::itemType(const QUuid& id) const
{
	if (!itemExists(id))
		return 0;
	int itemNumber = d->m_idHash[id]; //the numerical index of this item
	return d->m_typeHash.value(itemNumber, 0);
}

QString Palapeli::GameStorage::itemFilePath(const QUuid& id) const
{
	if (!itemExists(id))
		return QString();
	int itemNumber = d->m_idHash[id]; //the numerical index of this item
	return d->m_filePathTemplate.arg(id.toString()).arg(d->m_extHash.value(itemNumber, QString()));
}

QString Palapeli::GameStorage::itemMetaData(const QUuid& id) const
{
	if (!itemExists(id))
		return QString();
	int itemNumber = d->m_idHash[id]; //the numerical index of this item
	return d->m_metaHash.value(itemNumber, QString());
}

bool Palapeli::GameStorage::itemSetMetaData(const QUuid& id, const QString& text) const
{
	if (!itemExists(id))
		return false;
	int itemNumber = d->m_idHash[id]; //the numerical index of this item
	if (d->m_metaHash.value(itemNumber, QString()) == text)
		return true;
	//write new value into internal storage
	d->m_metaHash[itemNumber] = text;
	//write new value
	if (text.isEmpty())
		d->m_metaGroup->deleteEntry(entryKey.arg(itemNumber));
	else
		d->m_metaGroup->writeEntry(entryKey.arg(itemNumber), text);
	d->m_config->sync();
	return true;
}

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

#include "library.h"
#include "librarydelegate.h"
#include "puzzleinfo.h"

#include <QMutex>
#include <KStandardDirs>
#include <ThreadWeaver/Weaver>

const QString mainConfigPath("palapeli/puzzlelibrary/%1.desktop");
const QString stateConfigPath("palapeli/puzzlelibrary/%1.cfg");
const QString imagePath("palapeli/puzzlelibrary/%1");

Palapeli::Library::Library()
	: QAbstractListModel()
	, m_weaver(new ThreadWeaver::Weaver)
{
	//find puzzles
	const QStringList puzzleFiles = KStandardDirs().findAllResources("data", mainConfigPath.arg("*"), KStandardDirs::NoDuplicates);
	foreach (const QString& mainConfigPath, puzzleFiles)
	{
		const QString identifier = mainConfigPath.section('/', -1, -1).section('.', 0, 0);
		Palapeli::PuzzleInfo* info = new Palapeli::PuzzleInfo(identifier);
		m_puzzles << info;
		Palapeli::PuzzleInfoLoader* loader = new Palapeli::PuzzleInfoLoader(info);
		m_weaver->enqueue(loader);
		connect(loader, SIGNAL(done(ThreadWeaver::Job*)), this, SLOT(loaderFinished(ThreadWeaver::Job*)));
	}
}

Palapeli::Library::~Library()
{
	m_weaver->finish();
	delete m_weaver;
}

/*static*/ QString Palapeli::Library::findFile(const QString& identifier, Palapeli::Library::FileType type)
{
	switch (type)
	{
		case Palapeli::Library::MainConfigFile:
			return KStandardDirs::locate("data", mainConfigPath.arg(identifier));
		case Palapeli::Library::StateConfigFile:
			return KStandardDirs::locateLocal("data", stateConfigPath.arg(identifier));
		case Palapeli::Library::ImageFile:
			return KStandardDirs::locate("data", imagePath.arg(identifier));
		default: //will never be reached, but g++ does not like it without a default
			return QString();
	}
}

int Palapeli::Library::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	return m_puzzles.count();
}

QVariant Palapeli::Library::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();
	if (index.row() >= m_puzzles.count())
		return QVariant();
	const Palapeli::PuzzleInfo* pInfo = m_puzzles[index.row()];
	if (!pInfo->mutex->tryLock()) //PuzzleInfoLoader is running but did not finish yet
		return QVariant();
	if (pInfo->name.isEmpty()) //PuzzleInfoLoader was not started yet
	{
		pInfo->mutex->unlock();
		return QVariant();
	}
	QVariant returnValue;
	switch (role)
	{
		case Qt::DisplayRole:
			returnValue = pInfo->name;
			break;
		case Qt::DecorationRole:
			returnValue = pInfo->thumbnail;
			break;
		case Palapeli::Library::IdentifierRole:
			returnValue = pInfo->identifier;
			break;
		case Palapeli::Library::CommentRole:
			returnValue = pInfo->comment;
			break;
		case Palapeli::Library::AuthorRole:
			returnValue = pInfo->author;
			break;
		case Palapeli::Library::PieceCountRole:
			returnValue = pInfo->pieceCount;
			break;
		case Palapeli::Library::ImageRole:
			returnValue = pInfo->image;
			break;
	}
	pInfo->mutex->unlock();
	return returnValue;
}

QModelIndex Palapeli::Library::puzzleIndex(const QString& identifier) const
{
	for (int i = 0; i < m_puzzles.count(); ++i)
	{
		Palapeli::PuzzleInfo* pInfo = m_puzzles[i];
		if (!pInfo->mutex->tryLock()) //PuzzleInfoLoader is running but did not finish yet
			return QModelIndex();
		bool rightItem = pInfo->identifier == identifier;
		pInfo->mutex->unlock();
		if (rightItem) {
			pInfo->mutex->unlock();
			return index(i, 0);
		}
	}
	return QModelIndex();
}

void Palapeli::Library::loaderFinished(ThreadWeaver::Job* job)
{
	Palapeli::PuzzleInfoLoader* loader = dynamic_cast<Palapeli::PuzzleInfoLoader*>(job);
	if (!loader)
		return;
	Palapeli::PuzzleInfo* info = loader->puzzleInfo();
	const int row = m_puzzles.indexOf(info);
	if (row == -1)
		return;
	const QModelIndex changedIndex = index(row, 0);
	emit dataChanged(changedIndex, changedIndex);
}

#include "library/library.moc"

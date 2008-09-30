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
#include "librarybase.h"
#include "librarydelegate.h"
#include "puzzleinfo.h"

#include <QMutex>
#include <KStandardDirs>
#include <ThreadWeaver/Weaver>

Palapeli::Library::Library(Palapeli::LibraryBase* base)
	: QAbstractListModel()
	, m_base(base)
	, m_weaver(new	 ThreadWeaver::Weaver)
{
	//load info for puzzles
	const QStringList identifiers = m_base->findEntries();
	foreach (const QString& identifier, identifiers)
	{
		Palapeli::PuzzleInfo* info = new Palapeli::PuzzleInfo(identifier);
		m_puzzles << info;
		Palapeli::PuzzleInfoLoader* loader = new Palapeli::PuzzleInfoLoader(info, base);
		m_weaver->enqueue(loader);
		connect(loader, SIGNAL(done(ThreadWeaver::Job*)), this, SLOT(loaderFinished(ThreadWeaver::Job*)));
	}
}

Palapeli::Library::~Library()
{
	m_weaver->finish();
	delete m_weaver;
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

Palapeli::LibraryBase* Palapeli::Library::base() const
{
	return m_base;
}

Palapeli::PuzzleInfo* Palapeli::Library::infoForPuzzle(const QString& identifier) const
{
	for (int i = 0; i < m_puzzles.count(); ++i)
	{
		m_puzzles[i]->mutex->lock();
		if (m_puzzles[i]->identifier == identifier)
		{
			m_puzzles[i]->mutex->unlock();
			return m_puzzles[i];
		}
		m_puzzles[i]->mutex->unlock();
	}
	return 0;
}

Palapeli::PuzzleInfo* Palapeli::Library::infoForPuzzle(const QModelIndex& index) const
{
	if (!index.isValid())
		return 0;
	if (index.row() >= m_puzzles.count())
		return 0;
	return m_puzzles[index.row()];
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

/***************************************************************************
 *   Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
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

#include "librarymodel.h"
#include "puzzle.h"

#include <QFile>
#include <QFileInfo>
#include <KCmdLineArgs>
#include <KIO/CopyJob>
#include <KMessageBox>
#include <KUrl>

Palapeli::LibraryModel::LibraryModel(QObject* parent)
	: QAbstractListModel(parent)
{
	//add all puzzles from the library
	QList<Palapeli::PuzzleLocation> libraryLocations = Palapeli::PuzzleLocation::listLibrary();
	if (libraryLocations.isEmpty())
		KMessageBox::information(0, QLatin1String("No puzzles found in the library. If you have not done this yet, open a terminal in the \"puzzles\" subdirectory of the Palapeli source tree, and execute the \"make-puzzles.sh\" script, then \"make install\" again.")); //TODO: remove before release
	foreach (const Palapeli::PuzzleLocation& libraryLocation, libraryLocations)
		m_puzzles << new Palapeli::Puzzle(libraryLocation);
	//if a puzzle file has been given on the command line, load that puzzle
	KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
	if (args->count() > 0)
		m_puzzles.prepend(new Palapeli::Puzzle(Palapeli::PuzzleLocation::fromUrl(args->url(0))));
	//NOTE: the MainWindow relies on this puzzle being located at row 0
}

Palapeli::LibraryModel::~LibraryModel()
{
	qDeleteAll(m_puzzles);
}

int Palapeli::LibraryModel::rowCount(const QModelIndex& parent) const
{
	return parent.isValid() ? 0 : m_puzzles.count();
}

QVariant Palapeli::LibraryModel::data(const QModelIndex& index, int role) const
{
	if (index.parent().isValid())
		return QVariant();
	Palapeli::Puzzle* puzzle = m_puzzles.value(index.row());
	if (!puzzle)
		return QVariant();
	if (!puzzle->readMetadata())
		return QVariant();
	switch (role)
	{
		case IdentifierRole:
			return puzzle->location().identifier();
		case NameRole: case Qt::DisplayRole:
			return puzzle->metadata()->name;
		case CommentRole:
			return puzzle->metadata()->comment;
		case AuthorRole:
			return puzzle->metadata()->author;
		case PieceCountRole:
			return puzzle->metadata()->pieceCount;
		case ThumbnailRole: case Qt::DecorationRole:
			return puzzle->metadata()->thumbnail;
		case IsFromLibraryRole:
			return puzzle->location().isFromLibrary();
		case IsDeleteableRole:
			return QFileInfo(puzzle->location().url().path()).isWritable();
		default:
			return QVariant();
	}
}

Qt::ItemFlags Palapeli::LibraryModel::flags(const QModelIndex& index) const
{
	Q_UNUSED(index)
	return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

Palapeli::Puzzle* Palapeli::LibraryModel::puzzle(const QModelIndex& index) const
{
	if (index.parent().isValid())
		return 0;
	return m_puzzles.value(index.row());
}

Palapeli::Puzzle* Palapeli::LibraryModel::puzzle(const QString& identifier) const
{
	foreach (Palapeli::Puzzle* puzzle, m_puzzles)
		if (puzzle->location().identifier() == identifier)
			return puzzle;
	return 0; //puzzle not found
}

void Palapeli::LibraryModel::importPuzzle(const KUrl& url)
{
	Palapeli::PuzzleLocation location = Palapeli::PuzzleLocation::fromUrl(url);
	if (location.isFromLibrary())
	{
		KMessageBox::sorry(0, i18n("This puzzle is already in your library."));
		return;
	}
	//check if a puzzle from the library would be overwritten
	foreach (Palapeli::Puzzle* puzzle, m_puzzles)
	{
		Palapeli::PuzzleLocation otherLoc = puzzle->location();
		if (otherLoc.isFromLibrary() && otherLoc.identifier() == location.identifier())
		{
			KMessageBox::sorry(0, i18n("A puzzle with the same name is already in your library. Delete that puzzle before importing the new puzzle."));
			return;
		}
	}
	//start to copy puzzle
	Palapeli::PuzzleLocation targetLoc = Palapeli::PuzzleLocation::fromLibrary(location.identifier());
	KIO::CopyJob* job = KIO::copy(location.url(), targetLoc.url());
	connect(job, SIGNAL(result(KJob*)), this, SLOT(importFinished(KJob*)));
}

void Palapeli::LibraryModel::importFinished(KJob* job)
{
	KIO::CopyJob* copyJob = static_cast<KIO::CopyJob*>(job);
	if (copyJob->error())
		copyJob->showErrorDialog();
	else
	{
		Palapeli::PuzzleLocation targetLoc = Palapeli::PuzzleLocation::fromUrl(copyJob->destUrl());
		beginInsertRows(QModelIndex(), m_puzzles.count(), m_puzzles.count());
		m_puzzles << new Palapeli::Puzzle(targetLoc);
		endInsertRows();
	}
}

#if 0
void Palapeli::LibraryModel::exportPuzzle(const QModelIndex& index, const KUrl& url)
{
}

void Palapeli::LibraryModel::exportFinished(KJob* job)
{
}
#endif

void Palapeli::LibraryModel::deletePuzzle(const QModelIndex& index)
{
	Palapeli::Puzzle* puzzle = this->puzzle(index);
	if (!puzzle)
		return; //The LibraryWidget already checks whether the puzzle can be deleted.
	beginRemoveRows(QModelIndex(), index.row(), index.row());
	QFile(puzzle->location().url().path()).remove();
	m_puzzles.removeAll(puzzle);
	delete puzzle;
	endRemoveRows();
}

#include "librarymodel.moc"

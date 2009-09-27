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
#include "puzzlereader.h"

#include <KConfigGroup>
#include <KDesktopFile>
#include <KStandardDirs>

Palapeli::LibraryModel::LibraryModel()
{
	QStringList puzzleFiles = KStandardDirs().findAllResources("data", "palapeli/puzzlelibrary/*.pala", KStandardDirs::NoDuplicates);
	foreach (const QString& puzzleFile, puzzleFiles)
	{
		const QString identifier = puzzleFile.section('/', -1, -1).section('.', 0, 0);
		m_puzzles << new Palapeli::PuzzleReader(identifier);
	}
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
	Palapeli::PuzzleReader* puzzleReader = m_puzzles.value(index.row());
	if (!puzzleReader)
		return QVariant();
	puzzleReader->loadMetadata(); //if necessary
	switch (role)
	{
		case IdentifierRole:
			return puzzleReader->identifier();
		case NameRole: case Qt::DisplayRole:
			return puzzleReader->name();
		case CommentRole:
			return puzzleReader->comment();
		case AuthorRole:
			return puzzleReader->author();
		case ThumbnailRole: case Qt::DecorationRole:
			return puzzleReader->thumbnail();
		default:
			return QVariant();
	}
}

Qt::ItemFlags Palapeli::LibraryModel::flags(const QModelIndex& index) const
{
	Q_UNUSED(index)
	return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

Palapeli::PuzzleReader* Palapeli::LibraryModel::puzzle(const QModelIndex& index) const
{
	if (index.parent().isValid())
		return 0;
	return m_puzzles.value(index.row());
}

#include "librarymodel.moc"

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

#include <KConfigGroup>
#include <KDesktopFile>
#include <KStandardDirs>

Palapeli::LibraryModel::LibraryModel()
{
	QStringList puzzleFiles = KStandardDirs().findAllResources("data", "palapeli/puzzlelibrary/*.pala", KStandardDirs::NoDuplicates);
	foreach (const QString& puzzleFile, puzzleFiles)
		m_puzzles << new Palapeli::Puzzle(puzzleFile);
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
	const KDesktopFile* puzzleManifest = puzzle->manifest();
	switch (role)
	{
		case IdentifierRole:
			return puzzle->identifier();
		case NameRole: case Qt::DisplayRole:
			return puzzleManifest->readName();
		case CommentRole:
			return puzzleManifest->readComment();
		case AuthorRole:
			return puzzleManifest->desktopGroup().readEntry("X-KDE-PluginInfo-Author", QString());
		case ThumbnailRole: case Qt::DecorationRole:
			return puzzle->thumbnail();
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

#include "librarymodel.moc"

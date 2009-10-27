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

#include "collection.h"
#include "puzzle.h"

#include <QUuid>

Palapeli::Collection::Collection()
{
}

Palapeli::Collection::~Collection()
{
	qDeleteAll(m_puzzles);
}

QModelIndex Palapeli::Collection::addPuzzle(Palapeli::Puzzle* puzzle, const QString& identifier)
{
	//NOTE: the subclass implementation has to make sure that the puzzle's metadata is available
	//generate an identifier if necessary
	QString theIdentifier = identifier.isEmpty() ? QUuid::createUuid().toString() : identifier;
	//add puzzle to list
	int newPos = m_puzzles.count();
	beginInsertRows(QModelIndex(), newPos, newPos);
	m_identifiers << theIdentifier;
	m_puzzles << puzzle;
	endInsertRows();
	return index(newPos);
}

void Palapeli::Collection::removePuzzle(const QModelIndex& index)
{
	//load puzzle from index
	QObject* puzzlePayload = index.data(PuzzleObjectRole).value<QObject*>();
	Palapeli::Puzzle* puzzle = qobject_cast<Palapeli::Puzzle*>(puzzlePayload);
	if (!puzzle)
		return;
	//locate puzzle in collection
	int pos = m_puzzles.indexOf(puzzle);
	if (pos < 0)
		return;
	//remove puzzle
	beginRemoveRows(QModelIndex(), pos, pos);
	m_identifiers.removeAt(pos);
	m_puzzles.removeAt(pos);
	delete puzzle;
	endRemoveRows();
}

QString Palapeli::Collection::name() const
{
	return m_name;
}

void Palapeli::Collection::setName(const QString& name)
{
	m_name = name;
}

bool Palapeli::Collection::canImportPuzzles() const
{
	return false;
}

QModelIndex Palapeli::Collection::importPuzzle(const Palapeli::Puzzle* const puzzle)
{
	Q_UNUSED(puzzle)
	return QModelIndex();
}

bool Palapeli::Collection::canDeletePuzzle(const QModelIndex& index) const
{
	Q_UNUSED(index)
	return false;
}

bool Palapeli::Collection::deletePuzzle(const QModelIndex& index)
{
	Q_UNUSED(index)
	return false;
}

int Palapeli::Collection::rowCount(const QModelIndex& parent) const
{
	return parent.isValid() ? 0 : m_puzzles.count();
}

QVariant Palapeli::Collection::data(const QModelIndex& index, int role) const
{
	if (!index.isValid() || index.parent().isValid())
		return QVariant();
	Palapeli::Puzzle* puzzle = m_puzzles.value(index.row());
	if (!puzzle || !puzzle->metadata())
		return QVariant();
	switch (role)
	{
		//invisible metadata
		case IdentifierRole:
			return m_identifiers.value(index.row());
		case IsDeleteableRole:
			return canDeletePuzzle(index);
		//object references
		case PuzzleObjectRole:
			return qVariantFromValue((QObject*) puzzle);
		//visible metadata
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
		default:
			return QVariant();
	}
}

#include "collection.moc"

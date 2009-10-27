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

#include "collection-filesystem.h"
#include "puzzle.h"

#include <KFileDialog>
#include <KLocalizedString>

Palapeli::FileSystemCollection::FileSystemCollection()
{
}

int Palapeli::FileSystemCollection::indexOfPuzzle(const KUrl& location) const
{
	for (int pos = 0; pos < rowCount(); ++pos)
	{
		QObject* puzzlePayload = index(pos).data(PuzzleObjectRole).value<QObject*>();
		Palapeli::Puzzle* puzzle = qobject_cast<Palapeli::Puzzle*>(puzzlePayload);
		if (puzzle && puzzle->location() == location)
			return pos;
	}
	return -1;
}

QModelIndex Palapeli::FileSystemCollection::providePuzzle(const KUrl& location)
{
	//has this puzzle already been read into this collection?
	int pos = indexOfPuzzle(location);
	if (pos >= 0)
		return index(pos);
	//read and validate puzzle
	Palapeli::Puzzle* puzzle = new Palapeli::Puzzle(location);
	if (!puzzle->readMetadata())
		return QModelIndex();
	//add puzzle
	return addPuzzleInternal(location, puzzle);
}

QModelIndex Palapeli::FileSystemCollection::addPuzzleInternal(const KUrl& location, Palapeli::Puzzle* puzzle)
{
	//find a sane identifier for this puzzle (must be unique during the session, but should also be the same for the same puzzle over the course of multiple sessions, in order to find savegames correctly)
	QString fileName = location.fileName();
#ifdef Q_OS_WIN
	char* disallowedChars = "\\:*?\"<>|"; //Windows forbids using these chars in filenames, so we'll strip them
	for (char* c = disallowedChars; *c; ++c)
		fileName.remove(*c);
#endif
	const QString identifierPattern = QString::fromLatin1("__FSC_%1_%2_").arg(fileName);
	int uniquifier = 0;
	while (m_usedIdentifiers.contains(identifierPattern.arg(uniquifier)))
		++uniquifier;
	const QString identifier = identifierPattern.arg(uniquifier);
	m_usedIdentifiers << identifier;
	//add to internal list
	return Palapeli::Collection::addPuzzle(puzzle, identifier);
}

bool Palapeli::FileSystemCollection::canImportPuzzles() const
{
	return true;
}

QModelIndex Palapeli::FileSystemCollection::importPuzzle(const Palapeli::Puzzle* const puzzle)
{
	if (!puzzle->metadata())
		return QModelIndex();
	//ask for a target file name
	const KUrl startLoc = QString::fromLatin1("kfiledialog:///palapeli-export/%1.puzzle").arg(puzzle->metadata()->name);
	const QString filter = i18nc("Filter for a file dialog", "*.puzzle|Palapeli puzzles (*.puzzle)");
	const KUrl location = KFileDialog::getSaveUrl(startLoc, filter);
	if (location.isEmpty())
		return QModelIndex(); //process aborted by user
	//create a copy of the given puzzle, and relocate it to the new location
	Palapeli::Puzzle* newPuzzle = new Palapeli::Puzzle(*puzzle);
	newPuzzle->setLocation(location);
	newPuzzle->write();
	//add to the internal storage for future use, and return model index
	return addPuzzleInternal(location, newPuzzle);
}

QModelIndexList Palapeli::FileSystemCollection::selectPuzzles()
{
	const QString filter = i18nc("Filter for a file dialog", "*.puzzle|Palapeli puzzles (*.puzzle)");
	KUrl::List urls = KFileDialog::getOpenUrls(KUrl("kfiledialog:///palapeli-import"), filter);
	QModelIndexList result;
	foreach (const KUrl& url, urls)
		if (!url.isEmpty())
		{
			QModelIndex index = providePuzzle(url);
			if (index.isValid())
				result << index;
		}
	return result;
}

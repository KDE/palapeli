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

#include "libraryview.h"
#include "librarydelegate.h"
#include "librarymodel.h"

Palapeli::LibraryView::LibraryView(QWidget* parent)
	: QListView(parent)
	, m_model(new Palapeli::LibraryModel(this))
{
	setModel(m_model);
	new Palapeli::LibraryDelegate(this);
}

Palapeli::LibraryModel* Palapeli::LibraryView::model() const
{
	return m_model;
}

void Palapeli::LibraryView::handlePlayButton()
{
	//The identifier of the according puzzle is saved in a dynamic property of the button.
	const QString puzzleIdentifier = sender()->property("PuzzleIdentifier").toString();
	Palapeli::Puzzle* puzzle = m_model->puzzle(puzzleIdentifier);
	if (puzzle)
		emit selected(puzzle);
}

#include "libraryview.moc"

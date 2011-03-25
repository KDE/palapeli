/***************************************************************************
 *   Copyright 2009-2011 Stefan Majewsky <majewsky@gmx.net>
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

#include "puzzle-old.h"
#include "puzzle.h"
#include "components.h"

#include <QtCore/QFutureWatcher>

const QSize Palapeli::PuzzleMetadata::ThumbnailBaseSize(64, 64);

Palapeli::OldPuzzle::OldPuzzle(Palapeli::PuzzleComponent* mainComponent, const KUrl& location, const QString& identifier)
	: m_puzzle(new Palapeli::Puzzle(mainComponent, location, identifier))
{
}

Palapeli::OldPuzzle::OldPuzzle(const KUrl& location, const QString& identifier)
	: m_puzzle(new Palapeli::Puzzle(new Palapeli::ArchiveStorageComponent, location, identifier))
{
}

Palapeli::OldPuzzle::OldPuzzle(const Palapeli::OldPuzzle& other, const QString& identifier)
	: QObject()
	, m_puzzle(new Palapeli::Puzzle(new Palapeli::CopyComponent(other.m_puzzle), other.m_puzzle->location(), identifier))
{
}

Palapeli::OldPuzzle::OldPuzzle(const Palapeli::PuzzleCreationContext& creationContext, const QString& identifier)
	: m_puzzle(new Palapeli::Puzzle(new Palapeli::CreationContextComponent(creationContext), KUrl(), identifier))
{
	readMetadata();
	readContents();
}

Palapeli::OldPuzzle::~OldPuzzle()
{
	delete m_puzzle;
}

KUrl Palapeli::OldPuzzle::location() const
{
	return m_puzzle->location();
}

void Palapeli::OldPuzzle::setLocation(const KUrl& location)
{
	m_puzzle->setLocation(location);
}

Palapeli::Puzzle* Palapeli::OldPuzzle::newPuzzle() const
{
	return m_puzzle;
}

const Palapeli::PuzzleMetadata* Palapeli::OldPuzzle::metadata() const
{
	const Palapeli::MetadataComponent* c = m_puzzle->component<Palapeli::MetadataComponent>();
	return c ? &c->metadata : 0;
}

const Palapeli::PuzzleContents* Palapeli::OldPuzzle::contents() const
{
	const Palapeli::ContentsComponent* c = m_puzzle->component<Palapeli::ContentsComponent>();
	return c ? &c->contents : 0;
}

bool Palapeli::OldPuzzle::readMetadata()
{
	const Palapeli::PuzzleComponent* component = m_puzzle->get(Palapeli::PuzzleComponent::Metadata);
	return (bool) component;
}

bool Palapeli::OldPuzzle::readContents()
{
	if (!readMetadata()) //legacy code might rely on metadata being available after readContents()
		return false;
	const Palapeli::PuzzleComponent* component = m_puzzle->get(Palapeli::PuzzleComponent::Contents);
	return (bool) component;
}

bool Palapeli::OldPuzzle::write()
{
	Palapeli::FutureWatcher* watcher = new Palapeli::FutureWatcher;
	connect(watcher, SIGNAL(finished()), SIGNAL(writeFinished()));
	connect(watcher, SIGNAL(finished()), watcher, SLOT(deleteLater()));
	watcher->setFuture(m_puzzle->get(Palapeli::PuzzleComponent::ArchiveStorage));
	//we don't know better and have to assume that the operation works out
	return true;
}

#include "puzzle-old.moc"

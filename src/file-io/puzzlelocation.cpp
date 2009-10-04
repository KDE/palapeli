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

#include "puzzlelocation.h"

#include <KGlobal>
#include <KStandardDirs>

Palapeli::PuzzleLocation Palapeli::PuzzleLocation::fromLibrary(const QString& identifier)
{
	Palapeli::PuzzleLocation loc;
	loc.m_identifier = identifier;
	static const QString pathTemplate = QString::fromLatin1("palapeli/puzzlelibrary/%1.puzzle");
	//locate the file (or use the URL to where puzzle files can be imported)
	loc.m_url = KStandardDirs::locate("data", pathTemplate.arg(identifier));
	if (loc.m_url.isEmpty())
		loc.m_url = KStandardDirs::locateLocal("data", pathTemplate.arg(identifier));
	loc.m_fromLibrary = true;
	return loc;
}

Palapeli::PuzzleLocation Palapeli::PuzzleLocation::fromUrl(const KUrl& url)
{
	Palapeli::PuzzleLocation loc;
	loc.m_url = url;
	loc.m_identifier = url.fileName(KUrl::IgnoreTrailingSlash).section('.', 0, 0);
	//be clever about whether this is a file from the library
	loc.m_fromLibrary = Palapeli::PuzzleLocation::fromLibrary(loc.m_identifier).m_url == loc.m_url;
	return loc;
}

QList<Palapeli::PuzzleLocation> Palapeli::PuzzleLocation::listLibrary()
{
	QList<Palapeli::PuzzleLocation> result;
	QStringList puzzleUrls = KGlobal::dirs()->findAllResources("data", "palapeli/puzzlelibrary/*.puzzle", KStandardDirs::NoDuplicates);
	foreach (const QString& puzzleUrl, puzzleUrls)
	{
		//essentially the same as Palapeli::PuzzleLocation::fromUrl, but we can save some cleverness ;-)
		Palapeli::PuzzleLocation loc;
		loc.m_url = puzzleUrl;
		loc.m_identifier = loc.m_url.fileName(KUrl::IgnoreTrailingSlash).section('.', 0, 0);
		loc.m_fromLibrary = true;
		result << loc;
	}
	return result;
}

bool Palapeli::PuzzleLocation::isFromLibrary() const
{
	return m_fromLibrary;
}

KUrl Palapeli::PuzzleLocation::url() const
{
	return m_url;
}

QString Palapeli::PuzzleLocation::identifier() const
{
	return m_identifier;
}

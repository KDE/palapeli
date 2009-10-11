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

#ifndef PALAPELI_PUZZLELOCATION_H
#define PALAPELI_PUZZLELOCATION_H

#include <KUrl>

namespace Palapeli
{
	class PuzzleLocation
	{
		public:
			PuzzleLocation();
			static Palapeli::PuzzleLocation fromLibrary(const QString& identifier);
			static Palapeli::PuzzleLocation fromUrl(const KUrl& url);
			static QList<Palapeli::PuzzleLocation> listLibrary();

			bool isEmpty() const;
			bool isFromLibrary() const;

			KUrl url() const; ///Returns the URL of the puzzle file (determined via KStandardDirs for library puzzles).
			QString identifier() const; ///Returns the identifier of the puzzle file (read from the filename if a URL is given).

			bool operator==(const Palapeli::PuzzleLocation& other) const;
		private:
			KUrl m_url;
			QString m_identifier;
			bool m_fromLibrary;
	};
}

#endif // PALAPELI_PUZZLELOCATION_H

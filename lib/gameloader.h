/***************************************************************************
 *   Copyright 2008 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef PALAPELI_GAMELOADER_H
#define PALAPELI_GAMELOADER_H

#include "macros.h"

#include <QObject>

namespace Palapeli
{

	class Engine;
	class GameLoaderPrivate;
	class Library;
	class PuzzleInfo;

	class PALAPELIBASE_EXPORT GameLoader : public QObject
	{
		Q_OBJECT
		public:
			GameLoader(Palapeli::Engine* engine, const Palapeli::PuzzleInfo* info, bool takeLibraryOwnership = false); //true means: library will be destroyed in destructor (used with libraries on LibraryArchiveBases)
			~GameLoader();

			const Palapeli::PuzzleInfo* info() const;
			bool isValid() const;
		public Q_SLOTS:
			void save();
		Q_SIGNALS:
			void finished();
		protected Q_SLOTS:
			void finishLoading();
		private:
			GameLoaderPrivate* const p;
	};

}

#endif // PALAPELI_GAMELOADER_H

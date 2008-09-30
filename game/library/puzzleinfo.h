/***************************************************************************
 *   Copyright (C) 2008 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef PALAPELI_PUZZLEINFO_H
#define PALAPELI_PUZZLEINFO_H

#include <QImage>
class QMutex;
#include <QString>
class KConfig;
class KDesktopFile;

#include <ThreadWeaver/Job>

namespace Palapeli
{

	class LibraryBase;

	struct PuzzleInfo
	{
		PuzzleInfo(const QString& identifier);
		PuzzleInfo& operator=(PuzzleInfo& other);
		~PuzzleInfo();

		QString identifier, name, comment, author, imageFile;
		QImage image, thumbnail;
		int pieceCount;

		QMutex* const mutex;
	};

	class PuzzleInfoLoader : public ThreadWeaver::Job
	{
		public:
			PuzzleInfoLoader(Palapeli::PuzzleInfo* puzzleInfo, Palapeli::LibraryBase* base);
			Palapeli::PuzzleInfo* puzzleInfo() const;
		protected:
			virtual void run();
		private:
			Palapeli::LibraryBase* const m_base;
			Palapeli::PuzzleInfo* const m_puzzleInfo;
			bool m_puzzleExists;
	};

}

#endif // PALAPELI_PUZZLEINFO_H

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

#ifndef PALAPELI_PUZZLESTREAM_H
#define PALAPELI_PUZZLESTREAM_H

#include "puzzlelocation.h"
#include "puzzlestructs.h"

namespace Palapeli
{
	class PuzzleStreamJob;

	class PuzzleStream : public QObject
	{
		Q_OBJECT
		public:
			PuzzleStream(const KUrl& url);

			Palapeli::PuzzleLocation location() const;
			Palapeli::PuzzleStreamMetadata* metadata() const;
		public Q_SLOTS:
			void refresh();
		Q_SIGNALS:
			void refreshed();
		private Q_SLOTS:
			void handleRefreshFinished();
		private:
			KUrl m_url;
			Palapeli::PuzzleStreamMetadata* m_metadata;
			Palapeli::PuzzleStreamJob* m_job;
	};
}

#endif // PALAPELI_PUZZLESTREAM_H

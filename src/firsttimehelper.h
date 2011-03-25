/***************************************************************************
 *   Copyright 2009, 2010 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef PALAPELI_FIRSTTIMEHELPER_H
#define PALAPELI_FIRSTTIMEHELPER_H

#include <QObject>
class QProgressBar;

namespace Palapeli
{
	///This whole class is a hack, which I only implement at all because feature freeze is just 5.5 hours from now.
	///It checks the standard collection, and if some puzzles have not yet been built, it calls libpala-puzzlebuilder to build them.
	///TODO: Of course, the right way would be to implement this feature directly in the collection, but this will have to wait until Palapeli 1.2 because of aforementioned feature freeze.
	class FirstTimeHelper : public QObject
	{
		Q_OBJECT
		public:
			FirstTimeHelper();
			bool isNecessary() const;
		public Q_SLOTS:
			void execute();
		private Q_SLOTS:
			void next();
		Q_SIGNALS:
			void done();
		private:
			struct Task
			{
				QString desktopPath, puzzlePath;
			};
			QList<Task> m_tasks;
			QProgressBar* m_bar;
	};
}

#endif // PALAPELI_FIRSTTIMEHELPER_H

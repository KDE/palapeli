/***************************************************************************
 *   Copyright (C) 2008 Felix Lemke <lemke.felix@ages-skripte.org>
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

#ifndef PALAPELI_MANAGER_H
#define PALAPELI_MANAGER_H

class QImage;
#include <QObject>
class QPointF;
class QRectF;
class KUrl;

namespace Palapeli
{

	class Library;
	class MainWindow;
	class ManagerPrivate;
	class Minimap;
	class Preview;
	struct PuzzleInfo;

	class Manager : public QObject
	{
		Q_OBJECT
		public:
			static Manager* self();
			bool init();

			const PuzzleInfo* puzzleInfo() const;
			//other objects (mostly user interface)
			Library* library() const;
			Minimap* minimap() const;
			Preview* preview() const;
			MainWindow* window() const;
		public Q_SLOTS:
			void pieceMoveFinished();
			void searchConnections();

			void loadGame(const Palapeli::PuzzleInfo* info, bool forceReload = false, bool takeLibraryOwnership = false);
		private Q_SLOTS:
			void finishGameLoading();
		Q_SIGNALS:
			void gameNameChanged(const QString& name);
			void interactionModeChanged(bool gameInteractionAvailable);
		private:
			Manager();
			~Manager();
			Q_DISABLE_COPY(Manager)

			ManagerPrivate* const p;
	};

}

//abbreviation for Palapeli::Manager::self()
inline Palapeli::Manager* ppMgr() { return Palapeli::Manager::self(); }

#endif //PALAPELI_MANAGER_H

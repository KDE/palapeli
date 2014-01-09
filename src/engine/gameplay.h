/***************************************************************************
 *   Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
 *   Copyright 2014 Ian Wadham <iandw.au@gmail.com>
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

#ifndef PALAPELI_GAMEPLAY_H
#define PALAPELI_GAMEPLAY_H

class QStackedWidget;

#include "../window/mainwindow.h"

namespace Palapeli
{
	/**
	 * This is the main class for Palapeli gameplay. It implements menu and
	 * toolbar actions and provides methods such as loading and starting a
	 * puzzle, managing piece-holders and showing a victory animation.
	 */

	class CollectionView;
	class Puzzle;
	class PuzzleTableWidget;
	class PuzzlePreview;

	class GamePlay : public QObject
	{
		Q_OBJECT
		public:
			GamePlay(MainWindow* mainWindow = 0);
			virtual ~GamePlay();
			void    init();
			CollectionView* collectionView()
					{ return m_collectionView; };
			PuzzleTableWidget* puzzleTable()
					{ return m_puzzleTable; };
		public Q_SLOTS:
			void playPuzzle(Palapeli::Puzzle* puzzle);
			void playPuzzleFile(const QString& path);
			void actionGoCollection();
			void actionTogglePreview();
			void actionCreate();
			void actionDelete();
			void actionImport();
			void actionExport();
			void toggleCloseUp();
			void configure();
		Q_SIGNALS:
			// void constrainedChanged(bool constrained);
			// void levelChanged(int level);
			// void zoomInRequest();
			// void zoomOutRequest();
		private:
			QStackedWidget*    m_centralWidget;
			CollectionView*    m_collectionView;
			PuzzleTableWidget* m_puzzleTable;
			PuzzlePreview*     m_puzzlePreview;
			MainWindow*        m_mainWindow;
	};
}

#endif // PALAPELI_GAMEPLAY_H

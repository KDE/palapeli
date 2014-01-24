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

#include <QTime>	// IDW test.

namespace Palapeli
{
	class CollectionView;
	class Puzzle;
	class PuzzleTableWidget;
	class PuzzlePreview;
	class Scene;
	class Piece;

	/**
	 * This is the main class for Palapeli gameplay. It implements menu and
	 * toolbar actions and provides methods such as loading and shuffling
	 * a puzzle, starting a puzzle, saving and restoring the state of the
	 * solution, managing piece-holders, reporting progress and showing
	 * a victory animation.
	 */

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
			void restartPuzzle();
			void configure();

			void positionChanged(int reduction);
		Q_SIGNALS:
			void reportProgress(int pieceCount, int originalCount);
			void victoryAnimationFinished();
		private Q_SLOTS:
			void loadPreview();
			void loadPuzzleFile();
			void loadNextPiece();
			void loadPiecePositions();
			void completeVisualsForNextPiece();
			void finishLoading();

			void playVictoryAnimation2();
			void playVictoryAnimation3();

			void updateSavedGame();
		private:
			void loadPuzzle();
			void playVictoryAnimation();
			void calculatePieceAreaSize();

			QStackedWidget*    m_centralWidget;
			CollectionView*    m_collectionView;
			PuzzleTableWidget* m_puzzleTable;
			PuzzlePreview*     m_puzzlePreview;
			MainWindow*        m_mainWindow;
			Puzzle*            m_puzzle;
			Scene*             m_puzzleTableScene;
			QList<Scene*>      m_sceneList;
			QSizeF             m_pieceAreaSize;
			QTimer*            m_savegameTimer;

			// Some stuff needed for loading puzzles.
			bool m_loadingPuzzle;
			QMap<int, Palapeli::Piece*> m_loadedPieces;
			int m_originalPieceCount;
			int m_currentPieceCount;
			QTime t;	// IDW test.
	};
}

#endif // PALAPELI_GAMEPLAY_H

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

class KConfig;

class QStackedWidget;

#include "../window/mainwindow.h"

#include <QTime>	// IDW test.

namespace Palapeli
{
	class CollectionView;
	class Puzzle;
	class PuzzleTableWidget;
	class PieceHolder;
	class PuzzlePreview;
	class View;
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
			explicit GamePlay(MainWindow* mainWindow = 0);
			virtual ~GamePlay();
			void    init();
			void    shutdown();
			CollectionView* collectionView()
					{ return m_collectionView; };
			PuzzleTableWidget* puzzleTable()
					{ return m_puzzleTable; };
			static const int LargePuzzle;
		public Q_SLOTS:
			void playPuzzle(Palapeli::Puzzle* puzzle);
			void playPuzzleFile(const QString& path);
			void actionGoCollection();
			void actionTogglePreview();
			void actionCreate();
			void actionDelete();
			void actionImport();
			void actionExport();
			void createHolder();
			void deleteHolder();
			void selectAll();
			void rearrangePieces();
			void actionZoomIn();
			void actionZoomOut();
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
			void finishLoading();

			void playVictoryAnimation2();
			void playVictoryAnimation3();

			void updateSavedGame();

			void changeSelectedHolder(PieceHolder* h);
			void teleport(Piece* piece, const QPointF& scenePos,
							View* view);
			void closeHolder(PieceHolder* h);
			void handleNewPieceSelection(View* view);

		private:
			static QString saveGamePath() { return QStringLiteral("collection/"); }
			static QString saveGameFileName(const QString &name) { return QStringLiteral("%1.save").arg(name); }
			void deletePuzzleViews();
			void loadPuzzle();
			void playVictoryAnimation();
			void calculatePieceAreaSize();
			void createHolder(const QString& name, bool sel = true);
			void transferPieces(const QList<Piece*> pieces,
					View* source, View* dest,
					const QPointF& scenePos = QPointF());
			void setPalapeliMode(bool playing);
			QList<Piece*> getSelectedPieces(View* v);

			void savePuzzleSettings(KConfig* savedConfig);
			void restorePuzzleSettings(KConfig* savedConfig);

			QStackedWidget*    m_centralWidget;
			CollectionView*    m_collectionView;
			PuzzleTableWidget* m_puzzleTable;
			PuzzlePreview*     m_puzzlePreview;
			MainWindow*        m_mainWindow;
			Puzzle*            m_puzzle;
			Scene*             m_puzzleTableScene;
			QList<View*>       m_viewList;
			QSizeF             m_pieceAreaSize;
			QTimer*            m_savegameTimer;
			PieceHolder*       m_currentHolder;
			PieceHolder*       m_previousHolder;

			// Some stuff needed for loading puzzles.
			bool m_loadingPuzzle;
			bool m_restoredGame;
			QMap<int, Palapeli::Piece*> m_loadedPieces;
			int m_originalPieceCount;
			int m_currentPieceCount;
			qreal m_sizeFactor;
			bool m_playing;
			bool m_canDeletePuzzle;
			bool m_canExportPuzzle;
			QTime t;	// IDW test.
	};
}

#endif // PALAPELI_GAMEPLAY_H

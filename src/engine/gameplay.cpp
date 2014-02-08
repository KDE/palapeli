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

#include "gameplay.h"

#include "../file-io/collection-view.h"
#include "../window/puzzletablewidget.h"
#include "../window/pieceholder.h"
#include "puzzlepreview.h"

#include "scene.h"
#include "view.h"
#include "piece.h"
#include "../file-io/puzzle.h"
#include "../file-io/components.h"
#include "../file-io/collection.h"
#include "../creator/puzzlecreator.h"

#include "../config/configdialog.h"
#include "settings.h"

#include <QtGui/QStackedWidget>
#include <QPointer>
#include <QPropertyAnimation>
#include <QFutureWatcher>
#include <QtCore/qmath.h>
#include <QInputDialog>
#include <KDE/KAction>
#include <KDE/KActionCollection>
#include <KDE/KLocalizedString>
#include <KDE/KMessageBox>
#include <KDE/KFileDialog>
#include <KStandardDirs>

// Use this because comma in type is not possible in foreach macro.
typedef QPair<int, int> DoubleIntPair;

//TODO: move LoadingWidget into here (stack into m_centralWidget)

const int Palapeli::GamePlay::LargePuzzle = 300;

Palapeli::GamePlay::GamePlay(MainWindow* mainWindow)
	: QObject(mainWindow)
	, m_centralWidget(new QStackedWidget)
	, m_collectionView(new Palapeli::CollectionView)
	, m_puzzleTable(new Palapeli::PuzzleTableWidget)
	, m_puzzlePreview(0)
	, m_mainWindow(mainWindow)
	, m_puzzle(0)
	, m_pieceAreaSize(QSizeF(1.0, 1.0))
	, m_savegameTimer(new QTimer(this))
	, m_loadingPuzzle(false)
	, m_originalPieceCount(0)
	, m_currentPieceCount(0)
	, m_sizeFactor(1.3)
	, m_currentHolder(0)
{
	m_puzzleTableScene = m_puzzleTable->view()->scene();
	m_viewList << m_puzzleTable->view();
	m_savegameTimer->setInterval(500); //write savegame twice per second at most
	m_savegameTimer->setSingleShot(true);
	connect(m_savegameTimer, SIGNAL(timeout()), this, SLOT(updateSavedGame()));
	connect(this, SIGNAL(reportProgress(int,int)),
		m_puzzleTable, SLOT(reportProgress(int,int)));
	connect(this, SIGNAL(victoryAnimationFinished()),
		m_puzzleTable->view(), SLOT(startVictoryAnimation()));
}

Palapeli::GamePlay::~GamePlay()
{
	deletePuzzleViews();
	delete m_puzzlePreview;
}

void Palapeli::GamePlay::deletePuzzleViews()
{
	while (! m_viewList.isEmpty()) {
		Palapeli::View* view = m_viewList.takeLast();
		Palapeli::Scene* scene = view->scene();
		qDebug() << "DISCONNECT SLOT(positionChanged(int))";
		disconnect(scene, SIGNAL(saveMove(int)),
			   this, SLOT(positionChanged(int)));
		scene->clearPieces();
		if (scene != m_puzzleTableScene) {
			qDebug() << "DELETING holder" << view->windowTitle();
			delete view;
		}
	}
}

void Palapeli::GamePlay::init()
{
	// Set up the collection view.
	m_collectionView->setModel(Palapeli::Collection::instance());
	connect(m_collectionView, SIGNAL(playRequest(Palapeli::Puzzle*)), SLOT(playPuzzle(Palapeli::Puzzle*)));

	// Set up the puzzle table.
	m_puzzleTable->showStatusBar(Settings::showStatusBar());

	// Set up the central widget.
	m_centralWidget->addWidget(m_collectionView);
	m_centralWidget->addWidget(m_puzzleTable);
	m_centralWidget->setCurrentWidget(m_collectionView);
	m_mainWindow->setCentralWidget(m_centralWidget);
}

//BEGIN action handlers

void Palapeli::GamePlay::playPuzzle(Palapeli::Puzzle* puzzle)
{
	t.start();	// IDW test. START the clock.
	qDebug() << "START playPuzzle(): elapsed 0";
	m_centralWidget->setCurrentWidget(m_puzzleTable);
	m_mainWindow->actionCollection()->
				action("view_collection")->setEnabled(true);
	m_mainWindow->actionCollection()->
				action("game_restart")->setEnabled(true);
	m_mainWindow->actionCollection()->
				action("view_preview")->setEnabled(true);
	// IDW TODO - Could be empty if we use "view_collection" and come back.
	m_puzzlePreview = new Palapeli::PuzzlePreview();

	if (m_loadingPuzzle || (!puzzle) || (m_puzzle == puzzle)) {
		if (m_puzzle == puzzle) {
			qDebug() << "RESUMING A PUZZLE.";
			// IDW TODO - Show piece-holders.
			// Check if puzzle has been completed.
			if (m_currentPieceCount == 1) {
				int result = KMessageBox::questionYesNo(
					m_mainWindow,
					i18n("You have finished the puzzle. Do you want to restart it now?"));
				if (result == KMessageBox::Yes) {
					restartPuzzle();
					return;
				}
			}
		}
		qDebug() << "NO LOAD: (m_puzzle == puzzle)"
			 << (m_puzzle == puzzle);
		qDebug() << "m_loadingPuzzle" << m_loadingPuzzle
			 << (puzzle ? "puzzle != 0" : "puzzle == 0");
		return;		// Already loaded, loading or failed to start.
	}
	m_puzzle = puzzle;
	qDebug() << "RESTART the clock: elapsed" << t.restart(); // IDW test.
	loadPuzzle();
	qDebug() << "Returned from loadPuzzle(): elapsed" << t.elapsed();

	// IDW TODO - There is no way to stop loading a puzzle and start loading
	//            another, although it is possible to do "view_collection"
	//            during loading, pick another puzzle and continue loading
	//            the first puzzle. That puzzle eventually gets the wrong
	//            title (the title of the second with the contents of the
	//            first). Maybe it will also get the wrong Preview.

	QTimer::singleShot(0, this, SLOT(loadPreview()));
}

void Palapeli::GamePlay::loadPreview()
{
	// IDW TODO - This WAS delaying the showing of the LoadingWidget. Now
	//            it is preventing the balls from moving for a few seconds.

	// Get metadata from archive (tar), to be sure of getting image data.
	// The config/palapeli-collectionrc file lacks image metadata (because
	// Palapeli must load the collection-list quickly at startup time).
	const Palapeli::PuzzleComponent* as =
		m_puzzle->get(Palapeli::PuzzleComponent::ArchiveStorage);
	const Palapeli::PuzzleComponent* cmd = (as == 0) ? 0 :
		as->cast(Palapeli::PuzzleComponent::Metadata);
	if (cmd) {
		// Load puzzle preview image from metadata.
		const Palapeli::PuzzleMetadata md =
			dynamic_cast<const Palapeli::MetadataComponent*>(cmd)->
			metadata;
		m_puzzlePreview->loadImageFrom(md);
		m_mainWindow->setCaption(md.name);	// Set main title.
	}

	m_puzzlePreview->setVisible(Settings::puzzlePreviewVisible());
	connect (m_puzzlePreview, SIGNAL(closing()),
		SLOT(actionTogglePreview()));	// Hide preview: do not delete.
}

void Palapeli::GamePlay::playPuzzleFile(const QString& path)
{
	const QString id = Palapeli::Puzzle::fsIdentifier(path);
	playPuzzle(new Palapeli::Puzzle(new Palapeli::ArchiveStorageComponent,
						path, id));
}

void Palapeli::GamePlay::actionGoCollection()
{
	// IDW TODO - Should this terminate puzzle loading? Should it be enabled
	//            when a puzzle is loading?
	m_centralWidget->setCurrentWidget(m_collectionView);
	m_mainWindow->actionCollection()->action("view_collection")->setEnabled(false);
	m_mainWindow->actionCollection()->action("game_restart")->setEnabled(false);
	m_mainWindow->actionCollection()->action("view_preview")->setEnabled(false);
	delete m_puzzlePreview;
	m_puzzlePreview = 0;
	m_mainWindow->setCaption(QString());
	// IDW TODO - Disable piece-holder actions.
	foreach (Palapeli::View* view, m_viewList) {
		if (view != m_puzzleTable->view()) {
			view->hide();
		}
	}
}

void Palapeli::GamePlay::actionTogglePreview()
{
	// This action is OK during puzzle loading.
	if (m_puzzlePreview) {
		m_puzzlePreview->toggleVisible();
		m_mainWindow->actionCollection()->action("view_preview")->
			setChecked(Settings::puzzlePreviewVisible());
	}
}

void Palapeli::GamePlay::actionCreate()
{
	// IDW TODO - Should this terminate puzzle loading? Should it be enabled
	//            when a puzzle is loading?
	QPointer<Palapeli::PuzzleCreatorDialog> creatorDialog(new Palapeli::PuzzleCreatorDialog);
	if (creatorDialog->exec())
	{
		if (!creatorDialog)
			return;
		Palapeli::Puzzle* puzzle = creatorDialog->result();
		if (!puzzle) {
			delete creatorDialog;
			return;
		}
		Palapeli::Collection::instance()->importPuzzle(puzzle);
		playPuzzle(puzzle);
	}
	delete creatorDialog;
}

void Palapeli::GamePlay::actionDelete()
{
	// IDW TODO - Should this terminate puzzle loading? Should it be enabled
	//            when a puzzle is loading?
	QModelIndexList indexes = m_collectionView->selectedIndexes();
	//ask user for confirmation
	QStringList puzzleNames;
	foreach (const QModelIndex& index, indexes)
		puzzleNames << index.data(Qt::DisplayRole).toString();
	const int result = KMessageBox::warningContinueCancelList(m_mainWindow, i18n("The following puzzles will be deleted. This action cannot be undone."), puzzleNames);
	if (result != KMessageBox::Continue)
		return;
	//do deletion
	Palapeli::Collection* coll = Palapeli::Collection::instance();
	foreach (const QModelIndex& index, indexes)
		coll->deletePuzzle(index);
}

void Palapeli::GamePlay::actionImport()
{
	// IDW TODO - Should this terminate puzzle loading? Should it be enabled
	//            when a puzzle is loading?
	const QString filter = i18nc("Filter for a file dialog", "*.puzzle|Palapeli puzzles (*.puzzle)");
	const QStringList paths = KFileDialog::getOpenFileNames(KUrl("kfiledialog:///palapeli-import"), filter);
	Palapeli::Collection* coll = Palapeli::Collection::instance();
	foreach (const QString& path, paths)
		coll->importPuzzle(path);
}

void Palapeli::GamePlay::actionExport()
{
	// IDW TODO - Should this terminate puzzle loading? Should it be enabled
	//            when a puzzle is loading?
	QModelIndexList indexes = m_collectionView->selectedIndexes();
	Palapeli::Collection* coll = Palapeli::Collection::instance();
	foreach (const QModelIndex& index, indexes)
	{
		Palapeli::Puzzle* puzzle = coll->puzzleFromIndex(index);
		if (!puzzle)
			continue;
		//get puzzle name (as an initial guess for the file name)
		puzzle->get(Palapeli::PuzzleComponent::Metadata).waitForFinished();
		const Palapeli::MetadataComponent* cmp = puzzle->component<Palapeli::MetadataComponent>();
		if (!cmp)
			continue;
		//ask user for target file name
		const QString startLoc = QString::fromLatin1("kfiledialog:///palapeli-export/%1.puzzle").arg(cmp->metadata.name);
		const QString filter = i18nc("Filter for a file dialog", "*.puzzle|Palapeli puzzles (*.puzzle)");
		const QString location = KFileDialog::getSaveFileName(KUrl(startLoc), filter);
		if (location.isEmpty())
			continue; //process aborted by user
		//do export
		coll->exportPuzzle(index, location);
	}
}

void Palapeli::GamePlay::createHolder()
{
	qDebug() << "GamePlay::createHolder() entered";
	bool OK;
	QString name = QInputDialog::getText(m_mainWindow,
		i18n("Create a piece holder"),
		i18n("Enter a short name (optional):"),
		QLineEdit::Normal, QString(""), &OK);
	if (! OK) {
		return;		// If CANCELLED, do not create a piece holder.
	}
	PieceHolder* h = new PieceHolder(m_pieceAreaSize, name);
	m_viewList << h;
	connect(h, SIGNAL(selected(PieceHolder*)),
		this, SLOT(changeSelectedHolder(PieceHolder*)));
	changeSelectedHolder(h);
	m_puzzleTable->view()->setFocus(Qt::OtherFocusReason);
	m_puzzleTable->activateWindow();	// Return focus to main window.
}

void Palapeli::GamePlay::deleteHolder()
{
	qDebug() << "GamePlay::deleteHolder() entered";
	if (m_currentHolder) {
		if (m_currentHolder->scene()->pieces().isEmpty()) {
			int count = m_viewList.count();
			m_viewList.removeOne(m_currentHolder);
			qDebug() << "m_viewList WAS" << count << "NOW" << m_viewList.count();
			delete m_currentHolder;
			m_currentHolder = 0;
		}
		else {
			KMessageBox::information(m_mainWindow,
				i18n("The selected piece holder must be empty "
				     "before you can delete it."));
		}
	}
	else {
		KMessageBox::information(m_mainWindow,
			i18n("You need to click on a piece holder to "
			     "select it before you can delete it."));
	}
}

void Palapeli::GamePlay::selectAll()
{
	qDebug() << "GamePlay::selectAll() entered";
}

void Palapeli::GamePlay::rearrangePieces()
{
	qDebug() << "GamePlay::rearrangePieces() entered";
}

void Palapeli::GamePlay::actionZoomIn()
{
	// IDW TODO - Make ZoomIn work for whichever view is active.
	m_puzzleTable->view()->zoomIn();
}

void Palapeli::GamePlay::actionZoomOut()
{
	// IDW TODO - Make ZoomOut work for whichever view is active.
	m_puzzleTable->view()->zoomOut();
}

void Palapeli::GamePlay::restartPuzzle()
{
	// IDW TODO - Should this terminate puzzle loading? Should it be enabled
	//            when a puzzle is loading?
	if (!m_puzzle) {
		return;	// If no puzzle was successfully loaded and started.
	}
	// Discard the *.save file.
	static const QString pathTemplate =
			QString::fromLatin1("collection/%1.save");
	QFile(KStandardDirs::locateLocal("appdata",
			pathTemplate.arg(m_puzzle->identifier()))).remove();
	// Load the puzzle and re-shuffle the pieces.
	loadPuzzle();
}

void Palapeli::GamePlay::toggleCloseUp()
{
	m_puzzleTable->view()->toggleCloseUp();
}

void Palapeli::GamePlay::configure()
{
	Palapeli::ConfigDialog().exec();
}

//END action handlers

void Palapeli::GamePlay::loadPuzzle()
{
	qDebug() << "START loadPuzzle()";
	m_loadingPuzzle = true;
	// Stop autosaving and progress-reporting and start the loading-widget.
	m_savegameTimer->stop(); // Just in case it is running ...
	emit reportProgress(0, 0);
	// Return to the event queue to start the loading-widget graphics ASAP.
	QTimer::singleShot(0, this, SLOT(loadPuzzleFile()));
	qDebug() << "END loadPuzzle()";
}

void Palapeli::GamePlay::loadPuzzleFile()
{
	// Clear all scenes, and delete any piece holders that exist.
	qDebug() << "Start clearing all scenes: elapsed" << t.elapsed();
	deletePuzzleViews();
	m_viewList << m_puzzleTable->view();	// Re-list the puzzle-table.
	qDebug() << "Finish clearing all scenes: elapsed" << t.elapsed();

	qDebug() << "Start loadPuzzleFile(): elapsed" << t.restart();
	// Begin loading the puzzle.
	// It is loaded asynchronously and processed one piece at a time.
	m_loadedPieces.clear();
	if (m_puzzle) {
		Palapeli::FutureWatcher* watcher = new Palapeli::FutureWatcher;
		connect(watcher, SIGNAL(finished()), SLOT(loadNextPiece()));
		connect(watcher, SIGNAL(finished()),
			watcher, SLOT(deleteLater()));
		watcher->setFuture(
			m_puzzle->get(Palapeli::PuzzleComponent::Contents));
	}
	qDebug() << "Finish loadPuzzleFile(): time" << t.restart();
}

void Palapeli::GamePlay::loadNextPiece()
{
	if (!m_puzzle)
		return;
	const Palapeli::ContentsComponent* component =
			m_puzzle->component<Palapeli::ContentsComponent>();
	if (!component)
		return;
	// Add pieces, but only one at a time.
	// PuzzleContents structure is defined in src/file-io/puzzlestructs.h.
	// We iterate over contents.pieces: key = pieceID, value = QImage.
	const Palapeli::PuzzleContents contents = component->contents;
	QMap<int, QImage>::const_iterator iterPieces = contents.pieces.begin();
	const QMap<int, QImage>::const_iterator iterPiecesEnd =
						contents.pieces.end();
	for (int pieceID = iterPieces.key(); iterPieces != iterPiecesEnd;
						pieceID = (++iterPieces).key())
	{
		if (m_loadedPieces.contains(pieceID))
			continue;	// Already loaded.

		// Create a Palapeli::Piece from its image, offsets and ID.
		// This also adds bevels, if required.
		Palapeli::Piece* piece = new Palapeli::Piece(
			iterPieces.value(), contents.pieceOffsets[pieceID]);
		piece->addRepresentedAtomicPieces(QList<int>() << pieceID);
		piece->addAtomicSize(iterPieces.value().size());
		// IDW test. qDebug() << "PIECE" << pieceID
		//                << "offset" << contents.pieceOffsets[pieceID]
		//                << "size" << iterPieces.value().size();
		m_loadedPieces[pieceID] = piece;
		piece->completeVisuals();	// Add a shadow, if required.

		// Continue with next piece or next stage, after event loop run.
		if (contents.pieces.size() > m_loadedPieces.size())
			QTimer::singleShot(0, this, SLOT(loadNextPiece()));
		else
			QTimer::singleShot(0, this, SLOT(loadPiecePositions()));
		return;
	}
}

void Palapeli::GamePlay::loadPiecePositions()
{
	qDebug() << "Finish loadNextPiece() calls: time" << t.restart();
	if (!m_puzzle)
		return;
	qDebug() << "loadPiecePositions():";
	m_originalPieceCount = m_loadedPieces.count();
	const Palapeli::PuzzleContents contents = m_puzzle->component<Palapeli::ContentsComponent>()->contents;
	//add piece relations
	foreach (const DoubleIntPair& relation, contents.relations) {
		Palapeli::Piece* firstPiece =
				m_loadedPieces.value(relation.first, 0);
		Palapeli::Piece* secondPiece =
				m_loadedPieces.value(relation.second, 0);
		firstPiece->addLogicalNeighbors(QList<Palapeli::Piece*>()
				<< secondPiece);
		secondPiece->addLogicalNeighbors(QList<Palapeli::Piece*>()
				<< firstPiece);
	}
	foreach (Palapeli::Piece* piece, m_loadedPieces) {
		// Add all pieces to the main scene at first and move them
		// later, when or if a save file is found and processed.
		m_puzzleTableScene->addPieceToList(piece);
	}
	calculatePieceAreaSize();
	// IDW TODO - Need to tell every scene about this - as they are created.
	m_puzzleTableScene->setPieceAreaSize(m_pieceAreaSize);

	//Is "savegame" available?
	static const QString pathTemplate = QString::fromLatin1("collection/%1.save");
	KConfig saveConfig(KStandardDirs::locateLocal("appdata", pathTemplate.arg(m_puzzle->identifier())));
	if (saveConfig.hasGroup("SaveGame"))
	{
		// IDW TODO - Show piece-holders. Enable piece-holder actions.

		// IDW TODO - Here pieces can get put in multiple scenes ...
		// Read piece positions from the SaveGame group.
		// The current positions of atomic pieces are listed. If
		// neighbouring pieces are joined, their position values are
		// identical and searchConnections(m_pieces) handles that by
		// calling on a MergeGroup object to join the pieces.

		qDebug() << "RESTORING SAVED PUZZLE.";
		KConfigGroup saveGroup(&saveConfig, "SaveGame");
		QMap<int, Palapeli::Piece*>::const_iterator i =
						m_loadedPieces.constBegin();
		const QMap<int, Palapeli::Piece*>::const_iterator end =
						m_loadedPieces.constEnd();
		for (int pieceID = i.key(); i != end; pieceID = (++i).key())
		{
			Palapeli::Piece* piece = i.value();
			// qDebug() << "Saved:" << pieceID << "pos"
				 // << saveGroup.readEntry(
				 // QString::number(pieceID), QPointF());
			piece->setPos(saveGroup.readEntry(
					QString::number(pieceID), QPointF()));
		}
		// IDW TODO - This needs to be done for each Scene.
		// Each scene re-merges pieces, as required, with no animation.
		m_puzzleTableScene->mergeLoadedPieces();
	}
	else
	{
		// Place pieces at nice positions.
		qDebug() << "GENERATING A NEW PUZZLE BY SHUFFLING.";
		// Step 1: determine maximum piece size.
		QSizeF pieceAreaSize = m_pieceAreaSize;
		pieceAreaSize *= m_sizeFactor;	// Allow more space for pieces.

		// Step 2: place pieces in a grid in random order.
		QList<Palapeli::Piece*> piecePool(m_loadedPieces.values());
		int nPieces = piecePool.count();
		if (nPieces >= LargePuzzle) {
			m_viewList << new PieceHolder(pieceAreaSize, "Hand");
		}
		Palapeli::ConfigDialog::SolutionSpace space =
			(nPieces < 20) ?  Palapeli::ConfigDialog::None :
				(Palapeli::ConfigDialog::SolutionSpace)
				Settings::solutionArea();

		// Find the size of the area required for the solution.
		const QRectF r = m_puzzleTableScene->piecesBoundingRect();
		int xResv = 0;
		int yResv = 0;
		if (space != Palapeli::ConfigDialog::None) {
			xResv = r.width()/pieceAreaSize.width() + 1.0;
			yResv = r.height()/pieceAreaSize.height() + 1.0;
		}

		// To get "a" pieces around the solution, both horizontally and
		// vertically, we need to solve for "a" in:
		//     (a+xResv) * (a+yResv) = piecePool.count() + xResv*yResv
		// or  a^2 + (xResv+yResv)*a - piecePool.count() = 0
		// Let q = qSqrt(((xResv+yResv)^2 + 4.piecePool.count())), then
		//     a = (-xResv-yResv +- q)/2, the solution of the quadratic.
		//
		// The positive root is a = (-xResv - yResv + q)/2. If there is
		// no solution area, xResv == yResv == 0 and the above equation
		// degenerates to "a" = sqrt(number of pieces), as in earlier
		// versions of Palapeli.

		qreal q  = qSqrt((xResv + yResv)*(xResv + yResv) + 4*nPieces);
		int a    = qRound((-xResv-yResv+q)/2.0);
		int xMax = xResv + a;

		// Set solution space for None or TopLeft: modify as required.
		int x1 = 0;
		int y1 = 0;
		if (space == Palapeli::ConfigDialog::TopRight) {
			x1 = a;
		}
		else if (space == Palapeli::ConfigDialog::Center) {
			x1 = a/2;
			y1 = a/2;
		}
		else if (space == Palapeli::ConfigDialog::BottomLeft) {
			y1 = a;
			// If the rows are uneven, push the partial row right.
			if ((nPieces + xResv*yResv) % xMax) {
				yResv++;
			}
		}
		else if (space == Palapeli::ConfigDialog::BottomRight) {
			x1 = a;
			y1 = a;
		}
		int x2 = x1 + xResv;
		int y2 = y1 + yResv;
		qDebug() << "Reserve:" << xResv << yResv << "position" << space;
		qDebug() << "Pieces" << piecePool.count() << "rect" << r
			 << "pieceAreaSize" << pieceAreaSize;
		qDebug() << "q" << q << "a" << a << "a/2" << a/2;
		qDebug() << "xMax" << xMax << "x1 y1" << x1 << y1
					   << "x2 y2" << x2 << y2;

		for (int y = 0; !piecePool.isEmpty(); ++y) {
			for (int x = 0; x < xMax && !piecePool.isEmpty(); ++x) {
				if ((x >= x1) && (x < x2) &&
				    (y >= y1) && (y < y2)) {
					continue;	// This space reserved.
				}
				// Select a random piece.
				Palapeli::Piece* piece = piecePool.takeAt(
						qrand() % piecePool.count());
				// Place it randomly in grid-cell (x, y).
				piece->setPlace(x, y, pieceAreaSize, true);
			}
		}
		// Save the generated puzzle.
		//
		// If the user goes back to the collection, without making any
		// moves, and looks at another puzzle, the generated puzzle
		// should not be shuffled again when he/she reloads: only when
		// he/she hits Restart Puzzle or chooses to resart a previously
		// solved puzzle.
		updateSavedGame();
	}
	foreach (Palapeli::View* view, m_viewList) {
		Palapeli::Scene* scene = view->scene();
		QRectF s = scene->piecesBoundingRect();
		qreal handleWidth = qMin(s.width(), s.height())/100.0;
		// Add margin for constraint_handles+spacer and setSceneRect().
		scene->addMargin(handleWidth, 0.5*handleWidth);
	}
	m_puzzleTableScene->addPieceItemsToScene();
	qDebug() << "Finish loadPiecePositions(): time" << t.restart();
	finishLoading();
}

void Palapeli::GamePlay::finishLoading()
{
	// qDebug() << "finishLoading(): Starting";
	m_puzzle->dropComponent(Palapeli::PuzzleComponent::Contents);
	// Start each scene and view.
	m_currentPieceCount = 0;
	foreach (Palapeli::View* view, m_viewList) {
		Palapeli::Scene* scene = view->scene();
		m_currentPieceCount = m_currentPieceCount +
					scene->pieces().size();
		// IDW TODO - Do this better. It's the VIEWS that need to know.
		scene->startPuzzle();
	}
	// Initialize external progress display.
	emit reportProgress(m_originalPieceCount, m_currentPieceCount);
	m_loadingPuzzle = false;
	// Check if puzzle has been completed.
	if (m_currentPieceCount == 1) {
		int result = KMessageBox::questionYesNo(m_mainWindow,
			i18n("You have finished the puzzle. Do you want to restart it now?"));
		if (result == KMessageBox::Yes) {
			restartPuzzle();
			return;
		}
	}
	// Connect moves and merges of pieces to autosaving and progress-report.
	// IDW TODO - Needs to be done for each scene.
	connect(m_puzzleTableScene, SIGNAL(saveMove(int)),
		this, SLOT(positionChanged(int)));
	qDebug() << "finishLoading(): time" << t.restart();
}

void Palapeli::GamePlay::calculatePieceAreaSize()
{
	m_pieceAreaSize = QSizeF(0.0, 0.0);
	foreach (Palapeli::Piece* piece, m_loadedPieces) {
		m_pieceAreaSize = m_pieceAreaSize.expandedTo
				(piece->sceneBareBoundingRect().size());
	}
	qDebug() << "m_pieceAreaSize =" << m_pieceAreaSize;
}

void Palapeli::GamePlay::playVictoryAnimation()
{
	m_puzzleTableScene->setConstrained(true);
	QPropertyAnimation* animation = new QPropertyAnimation
					(m_puzzleTableScene, "sceneRect", this);
	animation->setStartValue(m_puzzleTableScene->sceneRect());
	animation->setEndValue(m_puzzleTableScene->piecesBoundingRect());
	animation->setDuration(1000);
	connect(animation, SIGNAL(finished()),
		this, SLOT(playVictoryAnimation2()));
	animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void Palapeli::GamePlay::playVictoryAnimation2()
{
	m_puzzleTableScene->setSceneRect(m_puzzleTableScene->piecesBoundingRect());
	QTimer::singleShot(100, this, SIGNAL(victoryAnimationFinished()));
	// Give the View some time to play its part of the victory animation.
	QTimer::singleShot(1500, this, SLOT(playVictoryAnimation3()));
}

void Palapeli::GamePlay::playVictoryAnimation3()
{
	KMessageBox::information(m_mainWindow, i18n("Great! You have finished the puzzle."));
}

void Palapeli::GamePlay::positionChanged(int reduction)
{
	if (reduction) {
		qDebug() << "Reduction:" << reduction << "from" << m_currentPieceCount;
		bool victory = (m_currentPieceCount > 1) &&
			       ((m_currentPieceCount - reduction) <= 1);
		m_currentPieceCount = m_currentPieceCount - reduction;
		emit reportProgress(m_originalPieceCount, m_currentPieceCount);
		if (victory) {
			playVictoryAnimation();
		}
	}
	if (!m_savegameTimer->isActive())
		m_savegameTimer->start();
}

void Palapeli::GamePlay::updateSavedGame()
{
	static const QString pathTemplate = QString::fromLatin1("collection/%1.save");
	KConfig saveConfig(KStandardDirs::locateLocal("appdata", pathTemplate.arg(m_puzzle->identifier())));
	KConfigGroup saveGroup(&saveConfig, "SaveGame");
	// IDW TODO - Needs to be done for EACH scene, with unique string-IDs.
	foreach (Palapeli::Piece* piece, m_puzzleTableScene->pieces()) {
		const QPointF pos = piece->pos();
		foreach (int atomicPieceID, piece->representedAtomicPieces()) {
			saveGroup.writeEntry(QString::number(atomicPieceID), pos);
		}
	}
}

void Palapeli::GamePlay::changeSelectedHolder(PieceHolder* h)
{
	if (m_currentHolder && (m_currentHolder != h)) {
		m_currentHolder->setSelected(false);
	}
	m_currentHolder = h;
}

#include "gameplay.moc"

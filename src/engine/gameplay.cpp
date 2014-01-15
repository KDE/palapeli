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
#include <KDE/KAction>
#include <KDE/KActionCollection>
#include <KDE/KLocalizedString>
#include <KDE/KMessageBox>
#include <KDE/KFileDialog>
#include <KStandardDirs>

// Use this because comma in type is not possible in foreach macro.
typedef QPair<int, int> DoubleIntPair;

//TODO: move LoadingWidget into here (stack into m_centralWidget)

Palapeli::GamePlay::GamePlay(MainWindow* mainWindow)
	: QObject(mainWindow)
	, m_mainWindow(mainWindow)
	, m_centralWidget(new QStackedWidget)
	, m_collectionView(new Palapeli::CollectionView)
	, m_puzzleTable(new Palapeli::PuzzleTableWidget)
	, m_puzzlePreview(0)
	, m_puzzle(0)
	, m_savegameTimer(new QTimer(this))
	, m_originalPieceCount(0)
	, m_currentPieceCount(0)
{
	m_puzzleTableScene = m_puzzleTable->view()->scene();
	m_sceneList << m_puzzleTableScene;
	m_savegameTimer->setInterval(500); //write savegame twice per second at most
	m_savegameTimer->setSingleShot(true);
	connect(m_savegameTimer, SIGNAL(timeout()), this, SLOT(updateSavedGame()));
	connect(this, SIGNAL(reportProgress(int,int)),
		m_puzzleTable, SLOT(reportProgress(int,int)));
	connect(m_puzzleTableScene, SIGNAL(victory()),
		this, SLOT(playVictoryAnimation()));
	connect(this, SIGNAL(victoryAnimationFinished()),
		m_puzzleTable->view(), SLOT(startVictoryAnimation()));
}

Palapeli::GamePlay::~GamePlay()
{
	delete m_puzzlePreview;
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
	qDebug() << "START playPuzzle()";
	m_centralWidget->setCurrentWidget(m_puzzleTable);
	m_mainWindow->actionCollection()->
				action("go_collection")->setEnabled(true);
	m_mainWindow->actionCollection()->
				action("game_restart")->setEnabled(true);
	m_mainWindow->actionCollection()->
				action("toggle_preview")->setEnabled(true);
	// IDW TODO - Could be empty if we use "go_collection" and come back.
	m_puzzlePreview = new Palapeli::PuzzlePreview();

	if (m_loadingPuzzle || (!puzzle) || (m_puzzle == puzzle)) {
		// IDW TODO - If puzzle is already solved, we get no message.
		qDebug() << "NO LOAD: (m_puzzle == puzzle)"
			 << (m_puzzle == puzzle);
		return;		// Already loaded, loading or failed to start.
	}
	m_puzzle = puzzle;
	loadPuzzle();

	// IDW TODO - There is no way to stop loading a puzzle and start loading
	//            another, although it is possible to do "go_collection"
	//            during loading, pick another puzzle and continue loading
	//            the first puzzle. That puzzle eventually gets the wrong
	//            title (the title of the second with the contents of the
	//            first). Maybe it will also get the wrong Preview.

	// Get metadata from archive (tar), to be sure of getting image data.
	// The config/palapeli-collectionrc file lacks image metadata (because
	// Palapeli must load the collection-list quickly at startup time).
	const Palapeli::PuzzleComponent* as =
		puzzle->get(Palapeli::PuzzleComponent::ArchiveStorage);
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
	m_mainWindow->actionCollection()->action("go_collection")->setEnabled(false);
	m_mainWindow->actionCollection()->action("game_restart")->setEnabled(false);
	m_mainWindow->actionCollection()->action("toggle_preview")->setEnabled(false);
	delete m_puzzlePreview;
	m_puzzlePreview = 0;
	m_mainWindow->setCaption(QString());
}

void Palapeli::GamePlay::actionTogglePreview()
{
	// This action is OK during puzzle loading.
	if (m_puzzlePreview) {
		m_puzzlePreview->toggleVisible();
		m_mainWindow->actionCollection()->action("toggle_preview")->
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
	m_savegameTimer->stop(); // Just in case it is running ...
	m_loadingPuzzle = true;
	m_puzzleTableScene->setConstrained(false);
	// Turn off autosaving and progress-reporting during loading.
	// Clear all scenes, including any piece holders that exist.
	qDebug() << "DISCONNECT SLOT(positionChanged(int)";
	foreach (Palapeli::Scene* scene, m_sceneList) {
		disconnect(scene, SIGNAL(saveMove(int)),
			   this, SLOT(positionChanged(int)));
		scene->clearPieces();
	}
	emit reportProgress(0, 0);

	// Begin to load puzzle.
	m_loadedPieces.clear();
	m_pieces.clear();
	if (m_puzzle) {
		Palapeli::FutureWatcher* watcher = new Palapeli::FutureWatcher;
		connect(watcher, SIGNAL(finished()), SLOT(loadNextPiece()));
		connect(watcher, SIGNAL(finished()),
			watcher, SLOT(deleteLater()));
		watcher->setFuture(
			m_puzzle->get(Palapeli::PuzzleComponent::Contents));
	}
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
		Palapeli::Piece* piece = new Palapeli::Piece(
			iterPieces.value(), contents.pieceOffsets[pieceID]);
		piece->addRepresentedAtomicPieces(QList<int>() << pieceID);
		piece->addAtomicSize(iterPieces.value().size());

		// IDW TODO - Do we really need a "master" list in GamePlay?
		//
		//            NOTE: Each piece in m_loadedPieces has its ID as
		//            the first and only item in QList<int>
		//            representedAtomicPieces().
		// IDW TODO - Maybe we could have a QList of pieceID's.
		m_pieces << piece;
		m_loadedPieces[pieceID] = piece;

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
	if (!m_puzzle)
		return;
	qDebug() << "loadPiecePositions():";
	m_originalPieceCount = m_loadedPieces.count();
	const Palapeli::PuzzleContents contents = m_puzzle->component<Palapeli::ContentsComponent>()->contents;
	//add piece relations
	foreach (const DoubleIntPair& relation, contents.relations) {
		// IDW TODO - Can use m_loadedPieces.value(relation.first, 0).
		//            If using list, use m_pieces.at(relation.first).
		Palapeli::Piece* firstPiece = m_pieces[relation.first];
		Palapeli::Piece* secondPiece = m_pieces[relation.second];
		firstPiece->addLogicalNeighbors(QList<Palapeli::Piece*>()
				<< secondPiece);
		secondPiece->addLogicalNeighbors(QList<Palapeli::Piece*>()
				<< firstPiece);
	}
	// IDW TODO - Can use foreach (Palapeli::Piece* piece, m_loadedPieces)
	foreach (Palapeli::Piece* piece, m_pieces) {
		// IDW TODO - Palapeli::Scene::addPiece(), but which scene?
		//            We cannot know that yet. Maybe add all pieces to
		//            main scene at first and move them when/if a
		//            save file is processed.
		m_puzzleTableScene->addPiece(piece);
	}
	// IDW TODO - Assess all atomic pieces, not nec. in m_puzzleTableScene.
	m_puzzleTableScene->calculatePieceAreaSize();
	//Is "savegame" available?
	static const QString pathTemplate = QString::fromLatin1("collection/%1.save");
	KConfig saveConfig(KStandardDirs::locateLocal("appdata", pathTemplate.arg(m_puzzle->identifier())));
	if (saveConfig.hasGroup("SaveGame"))
	{
		// IDW TODO - Here pieces can get put in multiple scenes ...
		//read piece positions from savegame
		KConfigGroup saveGroup(&saveConfig, "SaveGame");
		QMap<int, Palapeli::Piece*>::const_iterator iterPieces = m_loadedPieces.constBegin();
		const QMap<int, Palapeli::Piece*>::const_iterator iterPiecesEnd = m_loadedPieces.constEnd();
		for (int pieceID = iterPieces.key(); iterPieces != iterPiecesEnd; pieceID = (++iterPieces).key())
		{
			Palapeli::Piece* piece = iterPieces.value();
			// qDebug() << "Saved:" << pieceID << "pos"
				 // << saveGroup.readEntry(
				 // QString::number(pieceID), QPointF());
			piece->setPos(saveGroup.readEntry(QString::number(pieceID), QPointF()));
		}
		// IDW TODO - This needs to be done for each Scene.
		// IDW TODO - The scene should use its OWN m_pieces, but here
		//            we can specify NO ANIMATION.
		m_puzzleTableScene->mergeLoadedPieces();
	}
	else
	{
		// IDW TODO - This applies ONLY to m_puzzleTableScene ...
		//place pieces at nice positions
		//step 1: determine maximum piece size
		QSizeF pieceAreaSize;
		// IDW TODO - Can use foreach (Palapeli::Piece* piece,
		//            m_loadedPieces) or calculatePieceAreaSize();
		foreach (Palapeli::Piece* piece, m_pieces)
			pieceAreaSize = pieceAreaSize.expandedTo(piece->sceneBareBoundingRect().size());
		pieceAreaSize *= 1.3; //more space for each piece
		//step 2: place pieces in a grid in random order
		// IDW TODO - Can use piecePool(m_loadedPieces.values()).
		QList<Palapeli::Piece*> piecePool(m_pieces);
		const int xCount = floor(qSqrt(piecePool.count()));
		for (int y = 0; !piecePool.isEmpty(); ++y)
		{
			for (int x = 0; x < xCount && !piecePool.isEmpty(); ++x)
			{
				//select random piece
				Palapeli::Piece* piece = piecePool.takeAt(qrand() % piecePool.count());
				//determine piece offset
				piece->setPos(QPointF());
				const QRectF br = piece->sceneBareBoundingRect();
				const QPointF pieceOffset = br.topLeft();
				const QSizeF pieceSize = br.size();
				//determine random position inside piece area
				const QPointF areaOffset(
					qrand() % (int)(pieceAreaSize.width() - pieceSize.width()),
					qrand() % (int)(pieceAreaSize.height() - pieceSize.height())
				);
				//move to desired position in (x,y) grid
				const QPointF gridBasePosition(x * pieceAreaSize.width(), y * pieceAreaSize.height());
				piece->setPos(gridBasePosition + areaOffset - pieceOffset);
			}
		}
	}
	//continue after eventloop run
	QTimer::singleShot(0, this, SLOT(completeVisualsForNextPiece()));
}

void Palapeli::GamePlay::completeVisualsForNextPiece()
{
	foreach (Palapeli::Piece* piece, m_pieces)
	{
		if (piece->completeVisuals())
		{
			//something had to be done -> continue with next piece after eventloop run
			QTimer::singleShot(0, this, SLOT(completeVisualsForNextPiece()));
			return;
		}
	}
	//no pieces without shadow left, or piece visuals completely disabled
	finishLoading();
}

void Palapeli::GamePlay::finishLoading()
{
	// qDebug() << "finishLoading(): Starting";
	m_puzzle->dropComponent(Palapeli::PuzzleComponent::Contents);
	// Start each scene and view.
	m_currentPieceCount = 0;
	foreach (Palapeli::Scene* scene, m_sceneList) {
		// qDebug() << "Start a scene and view";
		scene->setSceneRect(scene->piecesBoundingRect());
		m_currentPieceCount = m_currentPieceCount + scene->pieces().size();
		qDebug() << "LOADED" << m_loadedPieces.count() << "atomic" << m_currentPieceCount << "current";
		// IDW TODO - Do this better. It's the VIEWS that need to know.
		emit scene->startPuzzle();
	}
	// Initialize external progress display.
	emit reportProgress(m_originalPieceCount, m_currentPieceCount);
	emit puzzleStarted();	// IDW TODO - Who receives this signal?
	m_loadingPuzzle = false;
	// Check if puzzle has been completed.
	if (m_currentPieceCount == 1) {
		int result = KMessageBox::questionYesNo(m_mainWindow,
			i18n("You have finished the puzzle the last time. Do you want to restart it now?"));
		if (result == KMessageBox::Yes) {
			restartPuzzle();
			return;
		}
	}
	// Connect moves and merges of pieces to autosaving and progress-report.
	qDebug() << "CONNECT SLOT(positionChanged(int)";
	connect(m_puzzleTableScene, SIGNAL(saveMove(int)),
		this, SLOT(positionChanged(int))); // , Qt::UniqueConnection);
	qDebug() << "finishLoading(): Exiting";
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
	// qDebug() << "updateSavegame(): KConfigGroup found";
	// IDW TODO - Needs to be done for EACH scene, with unique string-IDs.
	foreach (Palapeli::Piece* piece, m_puzzleTableScene->pieces()) {
		// qDebug() << "updateSavedGame(): Get piece->pos()";
		const QPointF pos = piece->pos();
		// qDebug() << "updateSavedGame(): atomicPieces" << piece->representedAtomicPieces() << "pos" << pos;
		foreach (int atomicPieceID, piece->representedAtomicPieces()) {
			// qDebug() << "updateSavedGame(): Write" << atomicPieceID
				 // << "pos" << pos;
			saveGroup.writeEntry(QString::number(atomicPieceID), pos);
		}
	}
}

#include "gameplay.moc"

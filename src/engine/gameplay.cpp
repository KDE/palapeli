/*
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>
    SPDX-FileCopyrightText: 2014 Ian Wadham <iandw.au@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "gameplay.h"
#include "palapeli_debug.h"

#include "../file-io/collection-view.h"
#include "../window/puzzletablewidget.h"
#include "../window/pieceholder.h"
#include "puzzlepreview.h"

#include "scene.h"
#include "view.h"
#include "piece.h"
#include "texturehelper.h"
#include "interactormanager.h"
#include "../file-io/puzzle.h"
#include "../file-io/components.h"
#include "../file-io/collection.h"
#include "../creator/puzzlecreator.h"

#include "../config/configdialog.h"
#include "settings.h"
#include <QStackedWidget>
#include <QPointer>
#include <QPropertyAnimation>
#include <QFutureWatcher>
#include <QtMath>
#include <QStandardPaths>
#include <QInputDialog>
#include <QFileDialog>
#include <QRandomGenerator>
#include <KActionCollection>
#include <KLocalizedString>
#include <KMessageBox>

// Use this because comma in type is not possible in foreach macro.
typedef QPair<int, int> DoubleIntPair;

//TODO: move LoadingWidget into here (stack into m_centralWidget)

const int Palapeli::GamePlay::LargePuzzle = 300;

const QString HeaderSaveGroup     = QStringLiteral("-PalapeliSavedPuzzle");
const QString HolderSaveGroup     = QStringLiteral("Holders");
const QString LocationSaveGroup   = QStringLiteral("XYCo-ordinates");
const QString FormerSaveGroup     = QStringLiteral("SaveGame");
const QString AppearanceSaveGroup = QStringLiteral("Appearance");
const QString PreviewSaveGroup    = QStringLiteral("PuzzlePreview");

Palapeli::GamePlay::GamePlay(MainWindow* mainWindow)
	: QObject(mainWindow)
	, m_centralWidget(new QStackedWidget)
	, m_collectionView(new Palapeli::CollectionView)
	, m_puzzleTable(new Palapeli::PuzzleTableWidget)
	, m_puzzlePreview(nullptr)
	, m_mainWindow(mainWindow)
	, m_puzzle(nullptr)
	, m_pieceAreaSize(QSizeF(32.0, 32.0))	// Allow 1024 pixels initially.
	, m_savegameTimer(new QTimer(this))
	, m_currentHolder(nullptr)
	, m_previousHolder(nullptr)
	, m_loadingPuzzle(false)
	, m_restoredGame(false)
	, m_originalPieceCount(0)
	, m_currentPieceCount(0)
	, m_sizeFactor(1.3)
	, m_playing(false)
	, m_canDeletePuzzle(false)	// No puzzle selected at startup.
	, m_canExportPuzzle(false)
{
	m_puzzleTableScene = m_puzzleTable->view()->scene();
	m_viewList << m_puzzleTable->view();
	m_savegameTimer->setInterval(500); //write savegame twice per second at most
	m_savegameTimer->setSingleShot(true);
	connect(m_savegameTimer, &QTimer::timeout, this, &GamePlay::updateSavedGame);
	connect(this, &GamePlay::reportProgress,
		m_puzzleTable, &PuzzleTableWidget::reportProgress);
	connect(this, &GamePlay::victoryAnimationFinished,
		m_puzzleTable->view(), &View::startVictoryAnimation);
	connect(m_puzzleTable->view(), &View::teleport,
		this, &GamePlay::teleport);
}

Palapeli::GamePlay::~GamePlay()
{
	deletePuzzleViews();
	delete m_puzzlePreview;
}

void Palapeli::GamePlay::deletePuzzleViews()
{
	qCDebug(PALAPELI_LOG) << "ENTERED GamePlay::deletePuzzleViews() ...";
	while (! m_viewList.isEmpty()) {
		Palapeli::View* view = m_viewList.takeLast();
		Palapeli::Scene* scene = view->scene();
		qCDebug(PALAPELI_LOG) << "DISCONNECT SLOT(positionChanged(int))";
		disconnect(scene, &Scene::saveMove,
			   this, &GamePlay::positionChanged);
		qCDebug(PALAPELI_LOG) << "scene->clearPieces();";
		view->interactorManager()->resetActiveTriggers();
		scene->clearPieces();
		qCDebug(PALAPELI_LOG) << "if (scene != m_puzzleTableScene) {";
		if (scene != m_puzzleTableScene) {
			qCDebug(PALAPELI_LOG) << "DELETING holder" << view->windowTitle();
			delete view;
		}
	}
	m_currentHolder = nullptr;
	m_previousHolder = nullptr;
}

void Palapeli::GamePlay::init()
{
	// Set up the collection view.
	m_collectionView->setModel(Palapeli::Collection::instance(m_mainWindow));
	connect(m_collectionView, &CollectionView::playRequest, this, &GamePlay::playPuzzle);

	// Set up the puzzle table.
	m_puzzleTable->showStatusBar(Settings::showStatusBar());

	// Set up the central widget.
	m_centralWidget->addWidget(m_collectionView);
	m_centralWidget->addWidget(m_puzzleTable);
	m_centralWidget->setCurrentWidget(m_collectionView);
	m_mainWindow->setCentralWidget(m_centralWidget);
	// Get some current action states from the collection.
	m_canDeletePuzzle = m_mainWindow->actionCollection()->
				action(QStringLiteral("game_delete"))->isEnabled();
	m_canExportPuzzle = m_mainWindow->actionCollection()->
				action(QStringLiteral("game_export"))->isEnabled();
	// Enable collection actions and disable playing actions initially.
	setPalapeliMode(false);
}

void Palapeli::GamePlay::shutdown()
{
	qCDebug(PALAPELI_LOG) << "ENTERED Palapeli::GamePlay::shutdown()";
	// Make sure the last change is saved.
	if (m_savegameTimer->isActive()) {
		m_savegameTimer->stop();
		updateSavedGame();
	}
	// Delete piece-holders cleanly: no closeEvents in PieceHolder objects
	// and no messages about holders not being empty.
	deletePuzzleViews();
}

//BEGIN action handlers

void Palapeli::GamePlay::playPuzzle(Palapeli::Puzzle* puzzle)
{
	t.start();	// IDW test. START the clock.
	// we need to load the preview every time, although when the puzzle
	// is already loaded because the preview is destroyed in actionGoCollection()
	QTimer::singleShot(0, this, &GamePlay::loadPreview);

	qCDebug(PALAPELI_LOG) << "START playPuzzle(): elapsed 0";
	// Get some current action states from the collection.
	m_canDeletePuzzle = m_mainWindow->actionCollection()->
				action(QStringLiteral("game_delete"))->isEnabled();
	m_canExportPuzzle = m_mainWindow->actionCollection()->
				action(QStringLiteral("game_export"))->isEnabled();
	m_centralWidget->setCurrentWidget(m_puzzleTable);
	m_puzzlePreview = new Palapeli::PuzzlePreview(m_mainWindow);

	if (m_loadingPuzzle || (!puzzle) || (m_puzzle == puzzle)) {
		if (m_puzzle == puzzle) {
			qCDebug(PALAPELI_LOG) << "RESUMING A PUZZLE.";
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
			// True if same puzzle selected and not still loading.
			setPalapeliMode(! m_loadingPuzzle);
		}
		qCDebug(PALAPELI_LOG) << "NO LOAD: (m_puzzle == puzzle)"
			 << (m_puzzle == puzzle);
		qCDebug(PALAPELI_LOG) << "m_loadingPuzzle" << m_loadingPuzzle
			 << (puzzle ? "puzzle != 0" : "puzzle == 0");
		return;		// Already loaded, loading or failed to start.
	}
	m_puzzle = puzzle;
	qCDebug(PALAPELI_LOG) << "RESTART the clock: elapsed" << t.restart(); // IDW test.
	loadPuzzle();
	qCDebug(PALAPELI_LOG) << "Returned from loadPuzzle(): elapsed" << t.elapsed();

	// IDW TODO - There is no way to stop loading a puzzle and start loading
	//            another. The only option is to Quit or abort Palapeli.
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
	const Palapeli::PuzzleComponent* cmd = (as == nullptr) ? nullptr :
		as->cast(Palapeli::PuzzleComponent::Metadata);
	if (cmd) {
		// Load puzzle preview image from metadata.
		const Palapeli::PuzzleMetadata md =
			dynamic_cast<const Palapeli::MetadataComponent*>(cmd)->
			metadata;
		m_puzzlePreview->loadImageFrom(md);
		m_mainWindow->setCaption(md.name);	// Set main title.
		delete cmd;
	}

	m_puzzlePreview->setVisible(Settings::puzzlePreviewVisible());
	connect (m_puzzlePreview, &PuzzlePreview::closing,
		 this, &GamePlay::actionTogglePreview);	// Hide preview: do not delete.
	// sync with mainWindow
	m_mainWindow->actionCollection()->action(QStringLiteral("view_preview"))->
		setChecked(Settings::puzzlePreviewVisible());
}

void Palapeli::GamePlay::playPuzzleFile(const QString& path)
{
	const QString id = Palapeli::Puzzle::fsIdentifier(path);
	playPuzzle(new Palapeli::Puzzle(new Palapeli::ArchiveStorageComponent,
						path, id));
}

void Palapeli::GamePlay::actionGoCollection()
{
	m_centralWidget->setCurrentWidget(m_collectionView);
	delete m_puzzlePreview;
	m_puzzlePreview = nullptr;
	m_mainWindow->setCaption(QString());
	// IDW TODO - Disable piece-holder actions.
	for (Palapeli::View* view : qAsConst(m_viewList)) {
		if (view != m_puzzleTable->view()) {
			view->hide();
		}
	}
	// Disable playing actions and enable collection actions.
	setPalapeliMode(false);
}

void Palapeli::GamePlay::actionTogglePreview()
{
	// This action is OK during puzzle loading.
	if (m_puzzlePreview) {
		m_puzzlePreview->toggleVisible();
		m_mainWindow->actionCollection()->action(QStringLiteral("view_preview"))->
			setChecked(Settings::puzzlePreviewVisible());
		// remember state
		updateSavedGame();
	}
}

void Palapeli::GamePlay::actionCreate()
{
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
	QModelIndexList indexes = m_collectionView->selectedIndexes();
	//ask user for confirmation
	QStringList puzzleNames;
	for (const QModelIndex& index : qAsConst(indexes))
		puzzleNames << index.data(Qt::DisplayRole).toString();
	const int result = KMessageBox::warningContinueCancelList(m_mainWindow, i18n("The following puzzles will be deleted. This action cannot be undone."), puzzleNames);
	if (result != KMessageBox::Continue)
		return;
	//do deletion
	Palapeli::Collection* coll = Palapeli::Collection::instance();

	// We cannot simply use a foreach here, because after deleting the first
	// puzzle, the rest of the indexes should no longer be used (model was
	// modified).  Ask again for the list of selected indexes after each
	// step instead.
	while (indexes.size() > 0)
	{
		coll->deletePuzzle(indexes.at(0));
		indexes = m_collectionView->selectedIndexes();
	}
}

void Palapeli::GamePlay::actionImport()
{
	const QString filter = i18nc("Filter for a file dialog", "Palapeli puzzles (*.puzzle)");
	const QStringList paths = QFileDialog::getOpenFileNames(m_mainWindow,
															i18n("Import Palapeli puzzles"),
															QString(),
															filter);
	Palapeli::Collection* coll = Palapeli::Collection::instance();
	for (const QString& path : paths)
		coll->importPuzzle(path);
}

void Palapeli::GamePlay::actionExport()
{
	const QModelIndexList indexes = m_collectionView->selectedIndexes();
	Palapeli::Collection* coll = Palapeli::Collection::instance();
	for (const QModelIndex& index : indexes)
	{
		Palapeli::Puzzle* puzzle = coll->puzzleFromIndex(index);
		if (!puzzle)
			continue;
		//get puzzle name (as an initial guess for the file name)
		puzzle->get(Palapeli::PuzzleComponent::Metadata);
		const Palapeli::MetadataComponent* cmp = puzzle->component<Palapeli::MetadataComponent>();
		if (!cmp)
			continue;
		//ask user for target file name
		const QString startLoc = QString::fromLatin1("%1.puzzle").arg(cmp->metadata.name);
		const QString filter = i18nc("Filter for a file dialog", "Palapeli puzzles (*.puzzle)");
		const QString location = QFileDialog::getSaveFileName(m_mainWindow,
															  i18n("Save Palapeli puzzles"),
															  startLoc,
															  filter);
		if (location.isEmpty())
			continue; //process aborted by user
		//do export
		coll->exportPuzzle(index, location);
	}
}

void Palapeli::GamePlay::createHolder()
{
	qCDebug(PALAPELI_LOG) << "GamePlay::createHolder() entered";
	bool OK;
	QString name = QInputDialog::getText(m_mainWindow,
		i18n("Create a piece holder"),
		i18n("Enter a short name (optional):"),
		QLineEdit::Normal, QString(), &OK);
	if (! OK) {
		return;		// If CANCELLED, do not create a piece holder.
	}
	createHolder(name);
	// Merges/moves in new holders add to the progress bar and are saved.
	Palapeli::View* view = m_viewList.last();
	view->setCloseUp(true);	// New holders start in close-up scale.
	connect(view->scene(), &Scene::saveMove,
		this, &GamePlay::positionChanged);
	connect(view, &View::teleport,
		this, &GamePlay::teleport);
	connect(view, &View::newPieceSelectionSeen,
		this, &GamePlay::handleNewPieceSelection);
}

void Palapeli::GamePlay::createHolder(const QString& name, bool sel)
{
	Palapeli::PieceHolder* h =
		new Palapeli::PieceHolder(m_mainWindow, m_pieceAreaSize, name);
	m_viewList << h;
	h->initializeZooming();			// Min. view 2x2 to 6x6 pieces.
	connect(h, &PieceHolder::selected,
		this, &GamePlay::changeSelectedHolder);
	connect (h, &PieceHolder::closing,
		this, &GamePlay::closeHolder);
	if (sel) {
		changeSelectedHolder(h);
	}
	else {
		h->setSelected(false);
	}
	m_puzzleTable->view()->setFocus(Qt::OtherFocusReason);
	m_puzzleTable->activateWindow();	// Return focus to main window.
	positionChanged(0);			// Save holder - a little later.
}

void Palapeli::GamePlay::deleteHolder()
{
	qCDebug(PALAPELI_LOG) << "GamePlay::deleteHolder() entered";
	if (m_currentHolder) {
		closeHolder(m_currentHolder);
	}
	else {
		KMessageBox::information(m_mainWindow,
			i18n("You need to click on a piece holder to "
			     "select it before you can delete it, or "
			     "you can just click on its Close button."));
	}
}

void Palapeli::GamePlay::closeHolder(Palapeli::PieceHolder* h)
{
	if (h->scene()->pieces().isEmpty()) {
		int count = m_viewList.count();
		m_viewList.removeOne(h);
		qCDebug(PALAPELI_LOG) << "m_viewList WAS" << count << "NOW" << m_viewList.count();
		m_currentHolder = nullptr;
		m_previousHolder = nullptr;
		h->deleteLater();
		positionChanged(0);	// Save change - a little later.
	}
	else {
		KMessageBox::information(m_mainWindow,
			i18n("The selected piece holder must be empty "
			     "before you can delete it."));
	}
}

void Palapeli::GamePlay::selectAll()
{
	qCDebug(PALAPELI_LOG) << "GamePlay::selectAll() entered";
	if (m_currentHolder) {
		const QList<Palapeli::Piece*> pieces =
					m_currentHolder->scene()->pieces();
		if (! pieces.isEmpty()) {
			for (Palapeli::Piece* piece : pieces) {
				piece->setSelected(true);
			}
			handleNewPieceSelection(m_currentHolder);
		}
		else {
			KMessageBox::information(m_mainWindow,
				i18n("The selected piece holder must contain "
				     "some pieces for 'Select all' to use."));
		}
	}
	else {
		KMessageBox::information(m_mainWindow,
			i18n("You need to click on a piece holder to "
			     "select it before you can select all the "
			     "pieces in it."));
	}
}

void Palapeli::GamePlay::rearrangePieces()
{
	qCDebug(PALAPELI_LOG) << "GamePlay::rearrangePieces() entered";
	QList<Palapeli::Piece*> selectedPieces;
	Palapeli::View* view = m_puzzleTable->view();
	selectedPieces = getSelectedPieces(view);
	if (selectedPieces.isEmpty()) {
		if (m_currentHolder) {
			view = m_currentHolder;
			selectedPieces = getSelectedPieces(view);
		}
	}
	if (selectedPieces.isEmpty()) {
		KMessageBox::information(m_mainWindow,
			i18n("To rearrange pieces, either the puzzle table "
			     "must have some selected pieces or there "
			     "must be a selected holder with some selected "
			     "pieces in it."));
		return;
	}
	QRectF bRect;
	for (Palapeli::Piece* piece : qAsConst(selectedPieces)) {
		bRect |= piece->sceneBareBoundingRect();
	}
	Palapeli::Scene* scene = view->scene();
	// If in a piece-holder and ALL pieces are selected, start at (0, 0).
	scene->initializeGrid(((view == m_currentHolder) &&
			(selectedPieces.count() == scene->pieces().count())) ?
				QPointF(0.0, 0.0) : bRect.topLeft());
	for (Palapeli::Piece* piece : qAsConst(selectedPieces)) {
		scene->addToGrid(piece);
	}
	if (view == m_currentHolder) {
		// Adjust the piece-holder's scene to frame the pieces.
		scene->setSceneRect(scene->extPiecesBoundingRect());
	}
	positionChanged(0);	// There is no attempt to merge pieces here.
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
	if (!m_puzzle) {
		return;	// If no puzzle was successfully loaded and started.
	}
	// Discard the *.save file.
	const QString puzzleLoc(
				QStandardPaths::locate(QStandardPaths::AppLocalDataLocation,
									   saveGamePath() + saveGameFileName(m_puzzle->identifier())));
	if (!puzzleLoc.isEmpty())
		QFile(puzzleLoc).remove();

	// Load the puzzle and re-shuffle the pieces.
	loadPuzzle();
}

void Palapeli::GamePlay::teleport(Palapeli::Piece* pieceUnderMouse,
				  const QPointF& scenePos, Palapeli::View* view)
{
	qCDebug(PALAPELI_LOG) << "GamePlay::teleport: pieceUnder" << (pieceUnderMouse != nullptr)
		 << "scPos" << scenePos
		 << "PuzzleTable?" << (view == m_puzzleTable->view())
		 << "CurrentHolder?" << (view == m_currentHolder);
	if (! m_currentHolder) {
		KMessageBox::information(m_mainWindow,
			i18n("You need to have a piece holder and click it to "
			     "select it before you can transfer pieces into or "
			     "out of it."));
		return;
	}
	bool puzzleTableClick = (view == m_puzzleTable->view());
	QList<Palapeli::Piece*> selectedPieces;
	if (puzzleTableClick) {
		if (pieceUnderMouse && (!pieceUnderMouse->isSelected())) {
			pieceUnderMouse->setSelected(true);
		}
		selectedPieces = getSelectedPieces(view);
                if (!selectedPieces.isEmpty()) {
			// Transfer from the puzzle table to a piece-holder.
			for (Palapeli::Piece* piece : qAsConst(selectedPieces)) {
				if (piece->representedAtomicPieces().count()
					> 6) {
					int ans = 0;
					ans = KMessageBox::questionYesNo (
						m_mainWindow,
						i18n("You have selected to "
						"transfer a large piece "
						"containing more than six "
						"small pieces to a holder. Do "
						"you really wish to do that?"));
					if (ans == KMessageBox::No) {
						return;
					}
				}
			}
			transferPieces(selectedPieces, view, m_currentHolder);
		}
		else {
			selectedPieces = getSelectedPieces(m_currentHolder);
			qCDebug(PALAPELI_LOG) << "Transfer from holder" << selectedPieces.count() << m_currentHolder->name();
			// Transfer from a piece-holder to the puzzle table.
                        if (!selectedPieces.isEmpty()) {
				transferPieces(selectedPieces, m_currentHolder,
						view, scenePos);
			}
			else {
				KMessageBox::information(m_mainWindow,
					i18n("You need to select one or more "
					     "pieces to be transferred out of "
					     "the selected holder or select "
					     "pieces from the puzzle table "
					     "to be transferred into it."));
			}
		}
	}
	else {
		if (m_previousHolder) {
			selectedPieces = getSelectedPieces(m_previousHolder);
			// Transfer from one piece-holder to another.
                        if (!selectedPieces.isEmpty()) {
				transferPieces(selectedPieces, m_previousHolder,
						view, scenePos);
			}
			else {
				KMessageBox::information(m_mainWindow,
					i18n("You need to select one or more "
					     "pieces to be transferred from "
					     "the previous holder into the "
					     "newly selected holder."));
			}
		}
		else {
			KMessageBox::information(m_mainWindow,
				i18n("You need to have at least two holders, "
				     "one of them selected and with selected "
				     "pieces inside it, before you can "
				     "transfer pieces to a second holder."));
		}
	}
	positionChanged(0);		// Save the transfer - a little later.
}

void Palapeli::GamePlay::handleNewPieceSelection(Palapeli::View* view)
{
	// De-select pieces on puzzle table, to prevent teleport bounce-back.
	Palapeli::View* m_puzzleTableView = m_puzzleTable->view();
	if (view != m_puzzleTableView) {	// Pieces selected in a holder.
		const auto selectedPieces = getSelectedPieces(m_puzzleTableView);
		for (Palapeli::Piece* piece : selectedPieces) {
			piece->setSelected(false);
		}
	}
}

void Palapeli::GamePlay::transferPieces(const QList<Palapeli::Piece*> &pieces,
					Palapeli::View* source,
					Palapeli::View* dest,
					const QPointF& scenePos)
{
	qCDebug(PALAPELI_LOG) << "ENTERED GamePlay::transferPieces(): pieces" << pieces.count() << "SourceIsTable" << (source == m_puzzleTable->view()) << "DestIsTable" << (dest == m_puzzleTable->view()) << "scenePos" << scenePos;
	source->scene()->dispatchPieces(pieces);
	if ((source != m_puzzleTable->view()) &&	// If empty holder.
		(source->scene()->pieces().isEmpty())) {
		source->scene()->initializeGrid(QPointF(0.0, 0.0));
	}

	bool destIsPuzzleTable = (dest == m_puzzleTable->view());
	if (destIsPuzzleTable) {
		m_puzzleTableScene->initializeGrid(scenePos);
	}
	Palapeli::Scene* scene = dest->scene();
	const auto sccenePieces = scene->pieces();
	for (Palapeli::Piece* piece : sccenePieces) {
		// Clear all previous selections in the destination scene.
		if (piece->isSelected()) {
			piece->setSelected(false);
		}
	}
	for (Palapeli::Piece* piece : pieces) {
		// Leave the new arrivals selected, connected and in a grid.
		scene->addPieceToList(piece);
		scene->addItem(piece);
		scene->addToGrid(piece);
		piece->setSelected(true);
		connect(piece, &Piece::moved,
			scene, &Scene::pieceMoved);
	}
	source->scene()->update();
	scene->setSceneRect(scene->extPiecesBoundingRect());
	if (! destIsPuzzleTable) {
		dest->centerOn(pieces.last()->sceneBareBoundingRect().center());
	}
}

void Palapeli::GamePlay::setPalapeliMode(bool playing)
{
	// Palapeli has three modes: playing, loading and managing a collection.
	// When playing, collection actions are disabled and playing actions are
	// enabled: vice versa when managing the collection. When loading a
	// puzzle, both sets of actions are disabled, because they cannot work
	// concurrently with loading (enPlaying and enCollection both false).

	const QString playingActions[] = {
            QStringLiteral("view_collection"),
            QStringLiteral("game_restart"),
            QStringLiteral("view_preview"),
            QStringLiteral("move_create_holder"),
            QStringLiteral("move_delete_holder"),
            QStringLiteral("move_select_all"),
            QStringLiteral("move_rearrange"),
            QStringLiteral("view_zoom_in"),
            QStringLiteral("view_zoom_out"),
        };
	const QString collectionActions[] = {
            QStringLiteral("game_new"),
            QStringLiteral("game_delete"),
            QStringLiteral("game_import"),
            QStringLiteral("game_export"),
        };
	bool enPlaying    = (! m_loadingPuzzle) && playing;
	bool enCollection = (! m_loadingPuzzle) && (! playing);

	for (const auto &actionId : playingActions) {
		m_mainWindow->actionCollection()->
			action(actionId)->setEnabled(enPlaying);
	}
	for (const auto &actionId : collectionActions) {
		m_mainWindow->actionCollection()->
			action(actionId)->setEnabled(enCollection);
	}
	// The collection view may enable or disable Delete and Export actions,
	// depending on what puzzle, if any, is currently selected.
	if (enCollection) {
		m_mainWindow->actionCollection()->
			action(QStringLiteral("game_delete"))->setEnabled(m_canDeletePuzzle);
		m_mainWindow->actionCollection()->
			action(QStringLiteral("game_export"))->setEnabled(m_canExportPuzzle);
	}
	m_playing = playing;
}

QList<Palapeli::Piece*> Palapeli::GamePlay::getSelectedPieces(Palapeli::View* v)
{
	qCDebug(PALAPELI_LOG) << "ENTERED GamePlay::getSelectedPieces(): PuzzleTable" << (v == m_puzzleTable->view());
	const QList<QGraphicsItem*> sel = v->scene()->selectedItems();
	QList<Palapeli::Piece*> pieces;
	for (QGraphicsItem* item : sel) {
		Palapeli::Piece* p = Palapeli::Piece::fromSelectedItem(item);
		if (p) {
			pieces << p;
		}
	}
	return pieces;
}

void Palapeli::GamePlay::configure()
{
	if (Palapeli::ConfigDialog().exec() == QDialog::Accepted) {
		if (m_playing) {
			qCDebug(PALAPELI_LOG) << "SAVING SETTINGS FOR THIS PUZZLE";
			updateSavedGame();	// Save current puzzle Settings.
		}
	}
}

//END action handlers

void Palapeli::GamePlay::loadPuzzle()
{
	qCDebug(PALAPELI_LOG) << "START loadPuzzle()";
	m_restoredGame = false;
	// Disable all collection and playing actions during loading.
	m_loadingPuzzle = true;
	setPalapeliMode(false);
	// Stop autosaving and progress-reporting and start the loading-widget.
	m_savegameTimer->stop(); // Just in case it is running ...
	Q_EMIT reportProgress(0, 0);
	// Is there a saved game?
	const QString puzzleLoc(
				QStandardPaths::locate(QStandardPaths::AppLocalDataLocation,
									   saveGamePath() + saveGameFileName(m_puzzle->identifier())));

	if (!puzzleLoc.isEmpty())
	{
		KConfig savedConfig(puzzleLoc, KConfig::SimpleConfig);
		if (savedConfig.hasGroup(AppearanceSaveGroup)) {
			// Get settings for background, shadows, etc. in this puzzle.
			restorePuzzleSettings(&savedConfig);
		}
	}
	// Return to the event queue to start the loading-widget graphics ASAP.
	QTimer::singleShot(0, this, &GamePlay::loadPuzzleFile);
	qCDebug(PALAPELI_LOG) << "END loadPuzzle()";
}

void Palapeli::GamePlay::loadPuzzleFile()
{
	// Clear all scenes, and delete any piece holders that exist.
	qCDebug(PALAPELI_LOG) << "Start clearing all scenes: elapsed" << t.elapsed();
	deletePuzzleViews();
	m_viewList << m_puzzleTable->view();	// Re-list the puzzle-table.
	qCDebug(PALAPELI_LOG) << "Finish clearing all scenes: elapsed" << t.elapsed();

	qCDebug(PALAPELI_LOG) << "Start loadPuzzleFile(): elapsed" << t.restart();
	// Begin loading the puzzle.
	// It is loaded asynchronously and processed one piece at a time.
	m_loadedPieces.clear();
	if (m_puzzle) {
		m_puzzle->get(Palapeli::PuzzleComponent::Contents);
		QTimer::singleShot(0, this, &Palapeli::GamePlay::loadNextPiece);
	}
	qCDebug(PALAPELI_LOG) << "Finish loadPuzzleFile(): time" << t.restart();
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
		// IDW test. qCDebug(PALAPELI_LOG) << "PIECE" << pieceID
		//                << "offset" << contents.pieceOffsets[pieceID]
		//                << "size" << iterPieces.value().size();
		m_loadedPieces[pieceID] = piece;
		piece->completeVisuals();	// Add a shadow, if required.

		// Continue with next piece or next stage, after event loop run.
		if (contents.pieces.size() > m_loadedPieces.size())
			QTimer::singleShot(0, this, &Palapeli::GamePlay::loadNextPiece);
		else
			QTimer::singleShot(0, this, &Palapeli::GamePlay::loadPiecePositions);
		return;
	}
}

void Palapeli::GamePlay::loadPiecePositions()
{
	qCDebug(PALAPELI_LOG) << "Finish loadNextPiece() calls: time" << t.restart();
	if (!m_puzzle)
		return;
	qCDebug(PALAPELI_LOG) << "loadPiecePositions():";
	m_originalPieceCount = m_loadedPieces.count();
	const Palapeli::PuzzleContents contents = m_puzzle->component<Palapeli::ContentsComponent>()->contents;
	//add piece relations
	for (const DoubleIntPair& relation : contents.relations) {
		Palapeli::Piece* firstPiece =
				m_loadedPieces.value(relation.first, nullptr);
		Palapeli::Piece* secondPiece =
				m_loadedPieces.value(relation.second, nullptr);
		firstPiece->addLogicalNeighbors(QList<Palapeli::Piece*>()
				<< secondPiece);
		secondPiece->addLogicalNeighbors(QList<Palapeli::Piece*>()
				<< firstPiece);
	}
	calculatePieceAreaSize();
	m_puzzleTableScene->setPieceAreaSize(m_pieceAreaSize);

	// Is there a saved game?
	const QString puzzleLoc(
				QStandardPaths::locate(QStandardPaths::AppLocalDataLocation,
									   saveGamePath() + saveGameFileName(m_puzzle->identifier())));
	// empty -> file not found -> no saved game
	bool oldFormat = false;
	m_restoredGame = false;
	int nHolders = 0;
	KConfig savedConfig(puzzleLoc, KConfig::SimpleConfig);	// here because needed inside 'if (m_restoredGame)'
	if (!puzzleLoc.isEmpty()) {
		if (savedConfig.hasGroup(HeaderSaveGroup)) {
			KConfigGroup headerGroup(&savedConfig, HeaderSaveGroup);
			nHolders = headerGroup.readEntry("N_Holders", 0);
			m_restoredGame = true;
		}
		else if (savedConfig.hasGroup(FormerSaveGroup)) {
			m_restoredGame = true;
			oldFormat = true;
		}
	}
	if (m_restoredGame)
	{
		// IDW TODO - Enable piece-holder actions.

		// Read piece positions from the LocationSaveGroup.
		// The current positions of atomic pieces are listed. If
		// neighbouring pieces are joined, their position values are
		// identical and searchConnections(m_pieces) handles that by
		// calling on a MergeGroup object to join the pieces.

		qCDebug(PALAPELI_LOG) << "RESTORING SAVED PUZZLE.";
		KConfigGroup holderGroup   (&savedConfig, HolderSaveGroup);
		KConfigGroup locationGroup (&savedConfig, oldFormat ?
			FormerSaveGroup : LocationSaveGroup);

		// Re-create the saved piece-holders, if any.
		m_currentHolder = nullptr;
		for (int groupID = 1; groupID <= nHolders; groupID++) {
			KConfigGroup holder (&savedConfig,
					QStringLiteral("Holder_%1").arg(groupID));
			// Re-create a piece-holder and add it to m_viewList.
			qCDebug(PALAPELI_LOG) << "RE-CREATE HOLDER"
				 << QStringLiteral("Holder_%1").arg(groupID) << "name"
				 << holder.readEntry("Name", QString());
			createHolder(holder.readEntry("Name", QString()),
				     holder.readEntry("Selected", false));
			// Restore the piece-holder's size and position.
			QRect r = holder.readEntry("Geometry", QRect());
			qCDebug(PALAPELI_LOG) << "GEOMETRY" << r;
			Palapeli::View* v = m_viewList.at(groupID);
			v->resize(r.size());
			int x = (r.left() < 0) ? 0 : r.left();
			int y = (r.top() < 0)  ? 0 : r.top();
			v->move(x, y);
		}

		// Move pieces to saved positions, in holders or puzzle table.
		qCDebug(PALAPELI_LOG) << "START POSITIONING PIECES";
		qCDebug(PALAPELI_LOG) << "Old format" << oldFormat << HolderSaveGroup << (oldFormat ? FormerSaveGroup : LocationSaveGroup);
		QMap<int, Palapeli::Piece*>::const_iterator i =
						m_loadedPieces.constBegin();
		const QMap<int, Palapeli::Piece*>::const_iterator end =
						m_loadedPieces.constEnd();
		for (int pieceID = i.key(); i != end; pieceID = (++i).key())
		{
			Palapeli::Piece* piece = i.value();
			const QString ID = QString::number(pieceID);
			const int group = oldFormat ? 0 :
					holderGroup.readEntry(ID, 0);
			const QPointF p = locationGroup.readEntry(ID, QPointF());
			// qCDebug(PALAPELI_LOG) << "Piece ID" << ID << "group" << group << "pos" << p;
			Palapeli::View* view = m_viewList.at(group);
			// qCDebug(PALAPELI_LOG) << "View" << (view != 0) << "Scene" << (view->scene() != 0);
			view->scene()->addPieceToList(piece);
			// qCDebug(PALAPELI_LOG) << "PIECE HAS BEEN ADDED TO SCENE's LIST";
			piece->setPos(p);
			// qCDebug(PALAPELI_LOG) << "PIECE HAS BEEN POSITIONED";
			// IDW TODO - Selecting/unselecting did not trigger a
			//            save. Needed to bring back a "dirty" flag.
			// IDW TODO - Same for all other saveable actions?
		}
		qCDebug(PALAPELI_LOG) << "FINISHED POSITIONING PIECES";
		// Each scene re-merges pieces, as required, with no animation.
		for (Palapeli::View* view : qAsConst(m_viewList)) {
			view->scene()->mergeLoadedPieces();
		}
	}
	else
	{
		// Place pieces at nice positions.
		qCDebug(PALAPELI_LOG) << "GENERATING A NEW PUZZLE BY SHUFFLING.";
		// Step 1: determine maximum piece size.
		QSizeF pieceAreaSize = m_pieceAreaSize;
		m_sizeFactor = 1.0 + 0.05 * Settings::pieceSpacing();
		qCDebug(PALAPELI_LOG) << "PIECE SPACING FACTOR" << m_sizeFactor;
		pieceAreaSize *= m_sizeFactor;	// Allow more space for pieces.

		// Step 2: place pieces in a grid in random order.
		QList<Palapeli::Piece*> piecePool(m_loadedPieces.values());
		int nPieces = piecePool.count();
		Palapeli::ConfigDialog::SolutionSpace space =
			(nPieces < 20) ?  Palapeli::ConfigDialog::None :
				(Palapeli::ConfigDialog::SolutionSpace)
				Settings::solutionArea();

		// Find the size of the area required for the solution.
		QRectF r;
		for (Palapeli::Piece* piece : qAsConst(piecePool)) {
			r |= piece->sceneBareBoundingRect();
		}
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
		qCDebug(PALAPELI_LOG) << "Reserve:" << xResv << yResv << "position" << space;
		qCDebug(PALAPELI_LOG) << "Pieces" << piecePool.count() << "rect" << r
			 << "pieceAreaSize" << pieceAreaSize;
		qCDebug(PALAPELI_LOG) << "q" << q << "a" << a << "a/2" << a/2;
		qCDebug(PALAPELI_LOG) << "xMax" << xMax << "x1 y1" << x1 << y1
					   << "x2 y2" << x2 << y2;

                auto *generator = QRandomGenerator::global();
		for (int y = 0; !piecePool.isEmpty(); ++y) {
			for (int x = 0; x < xMax && !piecePool.isEmpty(); ++x) {
				if ((x >= x1) && (x < x2) &&
				    (y >= y1) && (y < y2)) {
					continue;	// This space reserved.
				}
				// Select a random piece.
				Palapeli::Piece* piece = piecePool.takeAt(
                                                generator->bounded(piecePool.count()));
				// Place it randomly in grid-cell (x, y).
				const QPointF p0(0.0, 0.0);
				piece->setPlace(p0, x, y, pieceAreaSize, true);
				// Add piece to the puzzle table list (only).
				m_puzzleTableScene->addPieceToList(piece);
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
	// Add constraint_handles+spacer to puzzle table and setSceneRect().
	QRectF s = m_puzzleTableScene->piecesBoundingRect();
	qreal handleWidth = qMin(s.width(), s.height())/100.0;
	m_puzzleTableScene->addMargin(handleWidth, 0.5*handleWidth);
	// Add all the pieces to the puzzle table and piece-holder scenes.
	for (Palapeli::View* view : qAsConst(m_viewList)) {
		Palapeli::Scene* scene = view->scene();
		scene->addPieceItemsToScene();
		if (scene != m_puzzleTableScene) {
			// Expand the piece-holder sceneRects.
			scene->setSceneRect(scene->extPiecesBoundingRect());
		}
	}
	qCDebug(PALAPELI_LOG) << "Finish loadPiecePositions(): time" << t.restart();
	finishLoading();
}

void Palapeli::GamePlay::finishLoading()
{
	// qCDebug(PALAPELI_LOG) << "finishLoading(): Starting";
	m_puzzle->dropComponent(Palapeli::PuzzleComponent::Contents);
	// Start each scene and view.
	qCDebug(PALAPELI_LOG) << "COUNTING CURRENT PIECES";
	m_currentPieceCount = 0;
	for (Palapeli::View* view : qAsConst(m_viewList)) {
		Palapeli::Scene* scene = view->scene();
		m_currentPieceCount = m_currentPieceCount +
					scene->pieces().size();
		qCDebug(PALAPELI_LOG) << "Counted" << scene->pieces().size();
		if (view != m_puzzleTable->view()) {
			// Saved-and-restored holders start in close-up scale.
			view->setCloseUp(true);
		}
		else {
			qCDebug(PALAPELI_LOG) << "Puzzle table" << scene->pieces().size();
		}
	}
	// Initialize external progress display, hide loading widget, show view.
	Q_EMIT reportProgress(m_originalPieceCount, m_currentPieceCount);
	// Adjust zoom-levels, center the view, show autosave message if needed.
	m_puzzleTable->view()->puzzleStarted();
	if (!m_restoredGame && (m_originalPieceCount >= LargePuzzle)) {
		// New puzzle and a large one: create a default PieceHolder.
		createHolder(i18nc("For holding pieces", "Hand"));
		KMessageBox::information(m_mainWindow,
			i18nc("Hints for solving large puzzles",
			"You have just created a large puzzle: Palapeli has "
			"several features to help you solve it within the "
			"limited space on the desktop. They are described in "
			"detail in the Palapeli Handbook (on the Help menu). "
			"Here are just a few quick tips.\n\n"
			"Before beginning, it may be best not to use bevels or "
			"shadowing with large puzzles (see the Settings "
			"dialog), because they make loading slower and "
			"highlighting harder to see when the pieces in the "
			"view are very small.\n\n"
			"The first feature is the puzzle Preview (a picture of "
			"the completed puzzle) and a toolbar button to turn it "
			"on or off. If you hover over it with the mouse, it "
			"magnifies parts of the picture, so the window size "
			"you choose for the Preview can be quite small.\n\n"
			"Next, there are close-up and distant views of the "
			"puzzle table, which you can switch quickly by using "
			"a mouse button (default Middle-Click). In close-up "
			"view, use the empty space in the scroll bars to "
			"search through the puzzle pieces a 'page' at a time. "
			"You can adjust the two views by zooming in or out "
			"and your changes will be remembered.\n\n"
			"Then there is a space on the puzzle table reserved "
			"for building up the solution.\n\n"
			"Last but not least, there are small windows called "
			"'holders'. They are for sorting pieces into groups "
			"such as edges, sky or white house on left. You can "
			"have as many holders as you like and can give "
			"them names. You should already have one named "
			"'Hand', for carrying pieces from wherever you find "
			"them to the solution area.\n\n"
			"You use a special mouse click to transfer pieces into "
			"or out of a holder (default Shift Left-Click). First "
			"make sure the holder you want to use is active: it "
			"should have a blue outline. If not, click on it. To "
			"transfer pieces into the holder, select them on the "
			"puzzle table then do the special click to 'teleport' "
			"them into the holder. Or you can just do the special "
			"click on one piece at a time.\n\n"
			"To transfer pieces out of a holder, make "
			"sure no pieces are selected on the puzzle table, go "
			"into the holder window and select some pieces, using "
			"normal Palapeli mouse operations, then go back to the "
			"puzzle table and do the special click on an empty "
			"space where you want the pieces to arrive. Transfer "
			"no more than a few pieces at a time, to avoid "
			"collisions of pieces on the puzzle table.\n\n"
			"By the way, holders can do almost all the things the "
			"puzzle table and its window can do, including joining "
			"pieces to build up a part of the solution."),
			i18nc("Caption for hints", "Solving Large Puzzles"),
			QStringLiteral("largepuzzle-introduction"));
	}
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
	for (Palapeli::View* view : qAsConst(m_viewList)) {
		connect(view->scene(), &Scene::saveMove,
			this, &GamePlay::positionChanged);
		if (view != m_puzzleTable->view()) {
			connect(view, &View::teleport,
				this, &GamePlay::teleport);
			connect(view, &View::newPieceSelectionSeen,
				this, &GamePlay::handleNewPieceSelection);
		}
	}
	// Enable playing actions.
	m_loadingPuzzle = false;
	setPalapeliMode(true);
	qCDebug(PALAPELI_LOG) << "finishLoading(): time" << t.restart();
}

void Palapeli::GamePlay::calculatePieceAreaSize()
{
	m_pieceAreaSize = QSizeF(0.0, 0.0);
	for (Palapeli::Piece* piece : qAsConst(m_loadedPieces)) {
		m_pieceAreaSize = m_pieceAreaSize.expandedTo
				(piece->sceneBareBoundingRect().size());
	}
	qCDebug(PALAPELI_LOG) << "m_pieceAreaSize =" << m_pieceAreaSize;
}

void Palapeli::GamePlay::playVictoryAnimation()
{
	m_puzzleTableScene->setConstrained(true);
	QPropertyAnimation* animation = new QPropertyAnimation
					(m_puzzleTableScene, "sceneRect", this);
	animation->setStartValue(m_puzzleTableScene->sceneRect());
	animation->setEndValue(m_puzzleTableScene->extPiecesBoundingRect());
	animation->setDuration(1000);
	connect(animation, &QAbstractAnimation::finished,
		this, &GamePlay::playVictoryAnimation2);
	animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void Palapeli::GamePlay::playVictoryAnimation2()
{
	m_puzzleTableScene->setSceneRect(m_puzzleTableScene->extPiecesBoundingRect());
	QTimer::singleShot(100, this, &GamePlay::victoryAnimationFinished);
	// Give the View some time to play its part of the victory animation.
	QTimer::singleShot(1500, this, &GamePlay::playVictoryAnimation3);
}

void Palapeli::GamePlay::playVictoryAnimation3()
{
	KMessageBox::information(m_mainWindow, i18n("Great! You have finished the puzzle."));
}

void Palapeli::GamePlay::positionChanged(int reduction)
{
	if (reduction) {
		qCDebug(PALAPELI_LOG) << "Reduction:" << reduction << "from" << m_currentPieceCount;
		bool victory = (m_currentPieceCount > 1) &&
			       ((m_currentPieceCount - reduction) <= 1);
		m_currentPieceCount = m_currentPieceCount - reduction;
		Q_EMIT reportProgress(m_originalPieceCount, m_currentPieceCount);
		if (victory) {
			playVictoryAnimation();
		}
	}
	if (!m_savegameTimer->isActive())
		m_savegameTimer->start();
}

void Palapeli::GamePlay::updateSavedGame()
{
	QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) +
			QLatin1Char('/') + saveGamePath();
	QDir d(path);
	if (!d.exists())
		d.mkpath(path);

	KConfig savedConfig(path + saveGameFileName(m_puzzle->identifier()), KConfig::SimpleConfig);

	savePuzzleSettings(&savedConfig);

	// Save the positions of pieces and attributes of piece-holders.
	KConfigGroup headerGroup   (&savedConfig, HeaderSaveGroup);
	KConfigGroup holderGroup   (&savedConfig, HolderSaveGroup);
	KConfigGroup locationGroup (&savedConfig, LocationSaveGroup);

	headerGroup.writeEntry("N_Holders", m_viewList.count() - 1);

	int groupID = 0;
	for (Palapeli::View* view : qAsConst(m_viewList)) {
	    bool isHolder = (view != m_puzzleTable->view());
	    if (isHolder) {
		KConfigGroup holderDetails(&savedConfig,
			QStringLiteral("Holder_%1").arg(groupID));
		Palapeli::PieceHolder* holder =
			qobject_cast<Palapeli::PieceHolder*>(view);
		bool selected = (view == m_currentHolder);
		holderDetails.writeEntry("Name", holder->name());
		holderDetails.writeEntry("Selected", selected);
		holderDetails.writeEntry("Geometry",
			QRect(view->frameGeometry().topLeft(), view->size()));
	    }
	    const QList<Palapeli::Piece*> pieces = view->scene()->pieces();
	    for (Palapeli::Piece* piece : pieces) {
		const QPointF position = piece->pos();
		const auto atomicPieces = piece->representedAtomicPieces();
		for (int atomicPieceID : atomicPieces) {
		    const QString ID = QString::number(atomicPieceID);
		    locationGroup.writeEntry(ID, position);
		    if (isHolder) {
			holderGroup.writeEntry(ID, groupID);
		    }
		    else {
			holderGroup.deleteEntry(ID);
		    }
		}
	    }
	    groupID++;
	}
}

void Palapeli::GamePlay::savePuzzleSettings(KConfig* savedConfig)
{
	// Save the Appearance settings of the pieces and puzzle background.
	KConfigGroup settingsGroup (savedConfig, AppearanceSaveGroup);
	settingsGroup.writeEntry("PieceBevelsEnabled",
				Settings::pieceBevelsEnabled());
	settingsGroup.writeEntry("PieceShadowsEnabled",
				Settings::pieceShadowsEnabled());
	settingsGroup.writeEntry("PieceSpacing", Settings::pieceSpacing());
	settingsGroup.writeEntry("ViewBackground", Settings::viewBackground());
	settingsGroup.writeEntry("ViewBackgroundColor",
				Settings::viewBackgroundColor());
	settingsGroup.writeEntry("ViewHighlightColor",
				Settings::viewHighlightColor());
	Palapeli::ConfigDialog::SolutionSpace solutionArea =
				(Palapeli::ConfigDialog::SolutionSpace)
				Settings::solutionArea();
	settingsGroup.writeEntry("SolutionArea", (int)solutionArea);

	// Save the Preview settings.
	KConfigGroup previewGroup (savedConfig, PreviewSaveGroup);
	previewGroup.writeEntry("PuzzlePreviewGeometry",
				Settings::puzzlePreviewGeometry());
	previewGroup.writeEntry("PuzzlePreviewVisible",
				Settings::puzzlePreviewVisible());
}

void Palapeli::GamePlay::restorePuzzleSettings(KConfig* savedConfig)
{
	// Assume Palapeli::loadPuzzle() has tested if Appearance group exists.
	KConfigGroup settingsGroup(savedConfig, AppearanceSaveGroup);
	Settings::setPieceBevelsEnabled(settingsGroup.readEntry(
				"PieceBevelsEnabled", false));
	Settings::setPieceShadowsEnabled(settingsGroup.readEntry(
				"PieceShadowsEnabled", false));
	Settings::setPieceSpacing(settingsGroup.readEntry(
				"PieceSpacing", 6));
	Settings::setViewBackground(settingsGroup.readEntry(
				"ViewBackground", "background.svg"));
	Settings::setViewBackgroundColor(settingsGroup.readEntry(
				"ViewBackgroundColor", QColor(0xfff7eb)));
	Settings::setViewHighlightColor(settingsGroup.readEntry(
				"ViewHighlightColor", QColor(0x6effff)));
	Settings::setSolutionArea(settingsGroup.readEntry(
				"SolutionArea", 2));

	// Ask TextureHelper to re-draw background (but only after KConfigDialog
	// has written the settings, which might happen after this slot call).
	QTimer::singleShot(0, Palapeli::TextureHelper::instance(),
				&Palapeli::TextureHelper::readSettings);

	if (savedConfig->hasGroup(PreviewSaveGroup)) {
		KConfigGroup previewGroup(savedConfig, PreviewSaveGroup);
		Settings::setPuzzlePreviewGeometry(previewGroup.readEntry(
				"PuzzlePreviewGeometry", QRect(-1,-1,320,240)));
		Settings::setPuzzlePreviewVisible(previewGroup.readEntry(
				"PuzzlePreviewVisible", true));
	}
}

void Palapeli::GamePlay::changeSelectedHolder(Palapeli::PieceHolder* h)
{
	if (m_currentHolder && (m_currentHolder != h)) {
		m_previousHolder = m_currentHolder;
		m_currentHolder->setSelected(false);
	}
	m_currentHolder = h;
}



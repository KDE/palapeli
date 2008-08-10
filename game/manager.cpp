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

#include "manager.h"
#include "../lib/pattern.h"
#include "../lib/pattern-configuration.h"
#include "../lib/pattern-executor.h"
#include "../lib/pattern-trader.h"
#include "../storage/gamestorageattribs.h"
#include "../storage/gamestorage.h"
#include "../storage/gamestorageitem.h"
#include "autosaver.h"
#include "mainwindow.h"
#include "minimap.h"
#include "part.h"
#include "piece.h"
#include "piecerelation.h"
#include "preview.h"
#include "savegamemodel.h"
#include "statemanager.h"
#include "view.h"

#include <QFile>
#include <KConfig>
#include <KConfigGroup>
#include <KDesktopFile>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardDirs>
#include <KUrl>

namespace Palapeli
{

	struct ManagerPrivate
	{
		ManagerPrivate(Manager* manager);
		void init();
		~ManagerPrivate();

		bool canCreateGame(const KUrl& imageUrl, int patternIndex);
		bool canCreateGame(const QString& templateFile);
		bool canLoadGame(const QString& name);
		bool initGame();
		bool saveGame(const QString& name);

		Manager *m_manager;
		bool m_silent;

		//current game
		QImage m_image;
		QUuid m_imageId;
		QUuid m_gameId;
		QString m_gameName;
		PatternConfiguration* m_patternConfiguration;
		int m_estimatePieceCount; //used during slice process to show progress
		//game and UI objects
		Autosaver* m_autosaver;
		Minimap* m_minimap;
		QList<Part*> m_parts;
		QList<Piece*> m_pieces;
		Preview* m_preview;
		QList<PieceRelation> m_relations;
		SavegameModel* m_savegameModel;
		StateManager m_state;
		View* m_view;
		MainWindow* m_window;
	};

	namespace Strings
	{
		const QString AutosaveName("__palapeli_autosave_%1");
		//strings in .psg files
		const QString GeneralGroupKey("Palapeli");
		const QString PatternKey("Pattern");
		const QString GameNameKey("GameName");
		const QString PatternGroupKey("PatternArgs");
		const QString PiecesGroupKey("Pieces");
		const QString PositionKey("Position-%1");
	};

}

//BEGIN Palapeli::ManagerPrivate

Palapeli::ManagerPrivate::ManagerPrivate(Palapeli::Manager* manager)
	: m_manager(manager)
	, m_silent(false)
	, m_patternConfiguration(0)
	, m_autosaver(0)
	, m_minimap(0)
	, m_preview(new Palapeli::Preview)
	, m_savegameModel(0)
	, m_view(0)
	, m_window(0)
{
	//cleanup of now unused items (mostly images)
	Palapeli::GameStorage gs;
	Palapeli::GameStorageItems items = gs.queryItems(Palapeli::GameStorageAttributes() << new Palapeli::GameStorageNoDependencyAttribute);
	foreach (const Palapeli::GameStorageItem& item, items)
		gs.removeItem(item);
	//get list of savegames
	const Palapeli::GameStorageItems saveGames = gs.queryItems(Palapeli::GameStorageAttributes() << new Palapeli::GameStorageTypeAttribute(Palapeli::GameStorageItem::SavedGame));
	QStringList gameNames;
	foreach (const Palapeli::GameStorageItem& item, saveGames)
	{
		if (!item.metaData().startsWith(QLatin1String("__palapeli"), Qt::CaseSensitive))
			gameNames << item.metaData();
	}
	qSort(gameNames.begin(), gameNames.end(), Palapeli::SavegameModel::lessThan);
	//initialize savegame model
	m_savegameModel = new Palapeli::SavegameModel(gameNames);
	QObject::connect(m_manager, SIGNAL(savegameCreated(const QString&)), m_savegameModel, SLOT(savegameCreated(const QString&)));
	QObject::connect(m_manager, SIGNAL(savegameDeleted(const QString&)), m_savegameModel, SLOT(savegameDeleted(const QString&)));
}

void Palapeli::ManagerPrivate::init()
{
	//The Manager needs a valid pointer to this ManagerPrivate instance before the following objects can be initialized.
	m_autosaver = new Palapeli::Autosaver;
	m_minimap = new Palapeli::Minimap;
	m_view = new Palapeli::View;
	m_window = new Palapeli::MainWindow;
	//main window is deleted by Palapeli::ManagerPrivate::~ManagerPrivate because there are widely-spread references to m_view, and m_view will be deleted by m_window
	m_window->setAttribute(Qt::WA_DeleteOnClose, false);
}

Palapeli::ManagerPrivate::~ManagerPrivate()
{
	delete m_autosaver;
	delete m_minimap;
	foreach (Palapeli::Part* part, m_parts)
		delete part; //the pieces are deleted here
	delete m_preview;
	delete m_savegameModel;
	delete m_window; //the view is deleted here
}

bool Palapeli::ManagerPrivate::canCreateGame(const KUrl& imageUrl, int patternIndex)
{
	//load image into storage
	Palapeli::GameStorage storage;
	Palapeli::GameStorageItem item = storage.addItem(imageUrl, Palapeli::GameStorageItem::Image);
	if (!item.exists())
		return false;
	if (!m_image.load(item.filePath()))
	{
		KMessageBox::error(m_window, i18n("File seems not to be an image file."));
		return false;
	}
	m_imageId = item.id();
	//game not saved - no GameStorage ID available
	m_gameId = QUuid();
	//load pattern configuration
	m_patternConfiguration = Palapeli::PatternTrader::self()->configurationAt(patternIndex);
	return true;
}

bool Palapeli::ManagerPrivate::canCreateGame(const QString& templateFile)
{
	KDesktopFile df(templateFile);
	const QString path = templateFile.left(templateFile.lastIndexOf('/') + 1); //ends with a slash
	//find image
	const QString imageFile = path + df.readIcon();
	//find pattern configuration
	KConfigGroup generalGroup(&df, Palapeli::Strings::GeneralGroupKey);
	const QString patternName = generalGroup.readEntry(Palapeli::Strings::PatternKey, QString());
	Palapeli::PatternConfiguration* patternConfiguration = 0;
	for (int i = 0; i < Palapeli::PatternTrader::self()->configurationCount(); ++i)
	{
		Palapeli::PatternConfiguration* patternConfig = Palapeli::PatternTrader::self()->configurationAt(i);
		if (patternConfig->property("PatternName").toString() == patternName)
		{
			patternConfiguration = patternConfig; //pattern found
			break;
		}
	}
	if (patternConfiguration == 0)
		return false; //no pattern found
	//import image
	Palapeli::GameStorage storage;
	Palapeli::GameStorageItem item = storage.addItem(KUrl(imageFile), Palapeli::GameStorageItem::Image);
	if (!item.exists())
		return false;
	if (!m_image.load(item.filePath()))
	{
		KMessageBox::error(m_window, i18n("File seems not to be an image file."));
		return false;
	}
	m_imageId = item.id();
	//game not saved - no GameStorage ID available
	m_gameId = QUuid();
	//load pattern configuration
	m_patternConfiguration = patternConfiguration;
	KConfigGroup patternGroup(&df, Palapeli::Strings::PatternGroupKey);
	m_patternConfiguration->readArguments(&patternGroup);
	return true;
}

bool Palapeli::ManagerPrivate::canLoadGame(const QString& name)
{
	Palapeli::GameStorage storage;
	//find configuration for game
	Palapeli::GameStorageItems savegames = storage.queryItems(Palapeli::GameStorageAttributes() << new Palapeli::GameStorageTypeAttribute(Palapeli::GameStorageItem::SavedGame) << new Palapeli::GameStorageMetaAttribute(name));
	if (savegames.count() == 0)
		return false; //no image found
	//find image for game
	Palapeli::GameStorageItems images = storage.queryItems(Palapeli::GameStorageAttributes() << new Palapeli::GameStorageDependencyAttribute(savegames.at(0), Palapeli::GameStorageDependencyAttribute::SourceIsGiven));
	if (images.count() == 0)
		return false; //no image found
	//load configuration
	KConfig config(savegames.at(0).filePath());
	//find pattern
	KConfigGroup generalGroup(&config, Palapeli::Strings::GeneralGroupKey);
	const QString patternName = generalGroup.readEntry(Palapeli::Strings::PatternKey, QString());
	Palapeli::PatternConfiguration* patternConfiguration = 0;
	for (int i = 0; i < Palapeli::PatternTrader::self()->configurationCount(); ++i)
	{
		Palapeli::PatternConfiguration* patternConfig = Palapeli::PatternTrader::self()->configurationAt(i);
		if (patternConfig->property("PatternName").toString() == patternName)
		{
			patternConfiguration = patternConfig; //pattern found
			break;
		}
	}
	if (patternConfiguration == 0)
		return false; //no pattern found
	//load image
	if (!m_image.load(images.at(0).filePath()))
	{
		KMessageBox::error(m_window, i18n("Image file is missing or corrupted."));
		return false;
	}
	m_imageId = images.at(0).id();
	//save reference to configuration file, pattern configuration - this is not done earlier because the internal variables shouldn't change until everything has been located (to keep the internal state consistent)
	m_gameId = savegames.at(0).id();
	m_gameName = generalGroup.readEntry(Palapeli::Strings::GameNameKey, m_gameName);
	m_patternConfiguration = patternConfiguration;
	KConfigGroup patternGroup(&config, Palapeli::Strings::PatternGroupKey);
	m_patternConfiguration->readArguments(&patternGroup);
	return true;
}

bool Palapeli::ManagerPrivate::initGame()
{
	m_window->flushPuzzleProgress();
	//read piece positions if we're loading a game
	QList<QPointF> pieceBasePositions;
	if (!m_gameId.isNull())
	{
		Palapeli::GameStorage storage;
		KConfig config(storage.item(m_gameId).filePath());
		KConfigGroup piecesGroup(&config, Palapeli::Strings::PiecesGroupKey);
		for (int i = 0; piecesGroup.hasKey(Palapeli::Strings::PositionKey.arg(i)); ++i)
			pieceBasePositions << piecesGroup.readEntry(Palapeli::Strings::PositionKey.arg(i), QPointF());
	}
	//flush all variables
	foreach (Palapeli::Part* part, m_parts)
		delete part; //also deletes pieces
	m_parts.clear();
	m_pieces.clear();
	m_relations.clear();
	//configure scene and preview
	m_view->useScene(false); //disable the scene until the pieces have been built
	QRectF sceneRect(0, 0, 2 * m_image.width(), 2 * m_image.height());
	m_view->realScene()->setSceneRect(sceneRect);
	m_preview->setImage(m_image);
	//instantiate a pattern
	Palapeli::Pattern* pattern = m_patternConfiguration->createPattern();
	m_estimatePieceCount = 0; //by now, we do not know how much pieces to await
	if (!pieceBasePositions.isEmpty())
		pattern->loadPiecePositions(pieceBasePositions);
	QObject::connect(pattern, SIGNAL(estimatePieceCountAvailable(int)), m_manager, SLOT(estimatePieceCount(int)));
	QObject::connect(pattern, SIGNAL(pieceGenerated(const QImage&, const QRectF&, const QPointF&)),
		m_manager, SLOT(addPiece(const QImage&, const QRectF&, const QPointF&)));
	QObject::connect(pattern, SIGNAL(allPiecesGenerated()),
		m_manager, SLOT(endAddPiece()));
	QObject::connect(pattern, SIGNAL(relationGenerated(int, int)),
		m_manager, SLOT(addRelation(int, int)));
	//create pieces, parts, relations (in another thread)
	Palapeli::PatternExecutor* patternExec = new Palapeli::PatternExecutor(pattern);
	patternExec->setImage(m_image);
	QObject::connect(patternExec, SIGNAL(finished()), m_manager, SLOT(finishGameLoading()));
	patternExec->start();
	patternExec->wait();
	return true;
}

bool Palapeli::ManagerPrivate::saveGame(const QString& name)
{
	Palapeli::GameStorage gs;
	//find or create configuration file
	Palapeli::GameStorageItems configs = gs.queryItems(Palapeli::GameStorageAttributes() << new Palapeli::GameStorageTypeAttribute(Palapeli::GameStorageItem::SavedGame) << new Palapeli::GameStorageMetaAttribute(name));
	Palapeli::GameStorageItem configItem;
	if (configs.count() == 0)
	{
		configItem = gs.addItem("psg", Palapeli::GameStorageItem::SavedGame);
		configItem.setMetaData(name);
	}
	else
		configItem = configs.at(0);
	m_gameId = configItem.id();
	//open config file and write general information
	KConfig config(configItem.filePath());
	KConfigGroup generalGroup(&config, Palapeli::Strings::GeneralGroupKey);
	//write game name, pattern name, and configuration
	generalGroup.writeEntry(Palapeli::Strings::GameNameKey, m_gameName);
	QString patternName = m_patternConfiguration->property("PatternName").toString();
	generalGroup.writeEntry(Palapeli::Strings::PatternKey, patternName);
	KConfigGroup patternGroup(&config, Palapeli::Strings::PatternGroupKey);
	m_patternConfiguration->writeArguments(&patternGroup);
	//write piece positions
	KConfigGroup pieceGroup(&config, Palapeli::Strings::PiecesGroupKey);
	for (int i = 0; i < m_pieces.count(); ++i)
	{
		Palapeli::Piece* piece = m_pieces.at(i);
		pieceGroup.writeEntry(Palapeli::Strings::PositionKey.arg(i), piece->part()->basePosition());
	}
	//finalize configuration file
	config.sync();
	//create dependency from config to image
	gs.addDependency(configItem, gs.item(m_imageId));
	//reset autosaver to prevent autosaving immediately after this save
	m_autosaver->reset();
	return true;
}

//END Palapeli::ManagerPrivate

//BEGIN Palapeli::Manager

Palapeli::Manager::Manager()
	: QObject()
	, p(new Palapeli::ManagerPrivate(this))
{
	connect(this, SIGNAL(gameNameChanged(const QString&)), &p->m_state, SLOT(setGameName(const QString&)));
	p->m_state.setPersistent(true); //there is nothing that has not been saved (at the moment)
}

Palapeli::Manager::~Manager()
{
}

Palapeli::Manager* Palapeli::Manager::self()
{
	static Palapeli::Manager* theOneAndOnly = new Palapeli::Manager();
	return theOneAndOnly;
}

void Palapeli::Manager::init()
{
	static bool initialized = false; //make sure this happens only once
	if (initialized)
		return;
	p->init(); //This cannot be done automatically be done in the constructor: In this case, there would be another call to ppMgr() before theOneAndOnly in Palapeli::Manager::self is fully constructed, i.e. ppMgr() returns an uninitialized int.
	initialized = true;
	p->m_window->show();
}

//properties

int Palapeli::Manager::partCount() const
{
	return p->m_parts.count();
}

Palapeli::Part* Palapeli::Manager::partAt(int index) const
{
	return p->m_parts[index];
}

int Palapeli::Manager::pieceCount() const
{
	return p->m_pieces.count();
}

Palapeli::Piece* Palapeli::Manager::pieceAt(int index) const
{
	return p->m_pieces[index];
}

int Palapeli::Manager::relationCount() const
{
	return p->m_relations.count();
}

Palapeli::PieceRelation Palapeli::Manager::relationAt(int index) const
{
	return p->m_relations[index];
}

Palapeli::View* Palapeli::Manager::view() const
{
	return p->m_view;
}

Palapeli::Autosaver* Palapeli::Manager::autosaver() const
{
	return p->m_autosaver;
}

Palapeli::Minimap* Palapeli::Manager::minimap() const
{
	return p->m_minimap;
}

Palapeli::Preview* Palapeli::Manager::preview() const
{
	return p->m_preview;
}

Palapeli::SavegameModel* Palapeli::Manager::savegameModel() const
{
	return p->m_savegameModel;
}

Palapeli::MainWindow* Palapeli::Manager::window() const
{
	return p->m_window;
}

void Palapeli::Manager::updateGraphics()
{
	p->m_minimap->update();
}

//gameplay

void Palapeli::Manager::estimatePieceCount(int pieceCount)
{
	p->m_estimatePieceCount = qMax(0, pieceCount);
}

void Palapeli::Manager::addPiece(const QImage& image, const QRectF& positionInImage, const QPointF& sceneBasePosition)
{
	Palapeli::Piece* piece = new Palapeli::Piece(QPixmap::fromImage(image), positionInImage);
	p->m_pieces << piece;
	p->m_parts << new Palapeli::Part(piece);
	piece->part()->setBasePosition(sceneBasePosition);
	//keep application responsive
	int realPieceCount = p->m_pieces.count();
	int updateStep = qMax(p->m_estimatePieceCount / 15, 5);
	if ((realPieceCount + updateStep/2) % updateStep == 0) //do not redraw every time; this slows down the creation massively
	{
		int maxPieceCount = qMax(p->m_estimatePieceCount, realPieceCount);
		if (!p->m_silent)
			p->m_window->reportProgress(0, realPieceCount, maxPieceCount, i18np("1 piece generated", "%1 pieces generated", realPieceCount));
	}
}

void Palapeli::Manager::endAddPiece()
{
	//keep application responsive
	int maxPieceCount = qMax(p->m_estimatePieceCount, p->m_pieces.count());
	if (!p->m_silent)
		p->m_window->reportProgress(0, maxPieceCount + 1, maxPieceCount + 1, i18n("Finding neighbors"));
}

void Palapeli::Manager::addRelation(int piece1Id, int piece2Id)
{
	Palapeli::PieceRelation relation(p->m_pieces[piece1Id], p->m_pieces[piece2Id]);
	if (!p->m_relations.contains(relation))
		p->m_relations << relation;
}

void Palapeli::Manager::removePart(Part* part)
{
	p->m_parts.removeAll(part);
}

void Palapeli::Manager::pieceMoveFinished()
{
	searchConnections();
	p->m_autosaver->countMove();
	p->m_state.setPersistent(false); //current state was generated from user input (therefore not persistent)
}

void Palapeli::Manager::searchConnections()
{
	bool combinedSomething = false;
	foreach (Palapeli::PieceRelation rel, p->m_relations)
	{
		if (rel.piece1()->part() == rel.piece2()->part()) //already combined
			continue;
		if (rel.piecesInRightPosition())
		{
			rel.combine();
			combinedSomething = true;
		}
	}
	if (combinedSomething)
	{
		p->m_window->reportPuzzleProgress(p->m_pieces.count(), p->m_parts.count());
		updateGraphics();
	}
}

//game instances

void Palapeli::Manager::createGame(const KUrl& url, int patternIndex)
{
	if (!p->canCreateGame(url, patternIndex))
		return;
	p->m_state.setPersistent(false); //current state was generated from user input (therefore not persistent)
	p->m_gameName = QString();
	emit interactionModeChanged(false);
	p->initGame(); //partially asynchronous; slot finishGameLoading() will be called when finished
}

void Palapeli::Manager::createGame(const QString& templateName)
{
	//find configuration file for template
	static const QString templateLocation = QLatin1String("puzzlelibrary/%1.desktop");
	const QString templateFile = KStandardDirs::locate("appdata", templateLocation.arg(templateName));
	if (templateFile.isEmpty())
		return; //template does not exist
	//usual creation sequence
	if (!p->canCreateGame(templateFile))
		return;
	p->m_state.setPersistent(false); //current state was generated from user input (therefore not persistent)
	p->m_gameName = QString();
	emit interactionModeChanged(false);
	p->initGame(); //partially asynchronous; slot finishGameLoading() will be called when finished
}

void Palapeli::Manager::loadGame(const QString& name)
{
	if (!ensurePersistence())
		return;
	if (!p->canLoadGame(name))
		return;
	p->m_state.setPersistent(true); //current state was loaded from the hard disk (therefore persistent)
	p->m_gameName = name;
	emit interactionModeChanged(false);
	p->initGame(); //partially asynchronous; slot finishGameLoading() will be called when finished
}

void Palapeli::Manager::finishGameLoading()
{
	//restore relations
	searchConnections();
	//propagate changes
	if (!p->m_silent)
	{
		p->m_window->reportProgress(0, 1, 1, i18n("Game loaded."));
		p->m_window->flushProgress(2);
	}
	updateGraphics();
	p->m_view->useScene(true);
	p->m_autosaver->setEnabled(true);
	p->m_autosaver->reset();
	p->m_window->reportPuzzleProgress(p->m_pieces.count(), p->m_parts.count());
	emit interactionModeChanged(true);
	emit gameNameChanged(p->m_gameName);
}

bool Palapeli::Manager::saveGame(const QString& name)
{
	if (!p->m_patternConfiguration || p->m_pieces.empty())
		return false;
	if (name.startsWith(QLatin1String("__palapeli"), Qt::CaseSensitive))
	{
		KMessageBox::error(window(), i18n("Please choose another name. Names starting with \"__palapeli\" are reserved for internal use."));
		return false;
	}
	p->m_window->reportProgress(0, 1, 2, i18n("Saving game..."));
	emit interactionModeChanged(false);
	p->m_gameName = name;
	p->saveGame(name);
	p->m_state.setPersistent(true); //state has been written to disk and is therefore persistent
	emit gameNameChanged(name);
	emit savegameCreated(name);
	emit interactionModeChanged(true);
	p->m_window->reportProgress(0, 2, 2, i18n("Game saved."));
	p->m_window->flushProgress(2);
	return true;
}

bool Palapeli::Manager::autosaveGame()
{
	if (!p->m_patternConfiguration || p->m_pieces.empty())
		return false;
	p->m_window->reportProgress(0, 1, 2, i18n("Automatic backup..."));
	emit interactionModeChanged(false);
	p->m_silent = true;
	p->saveGame(Palapeli::Strings::AutosaveName.arg(p->m_state.id()));
	p->m_silent = false;
	emit interactionModeChanged(true);
	p->m_window->reportProgress(0, 2, 2, i18n("Automatic backup finished."));
	p->m_window->flushProgress(2);
	return true;
}

void Palapeli::Manager::deleteGame(const QString& name)
{
	Palapeli::GameStorage gs;
	Palapeli::GameStorageItems configs = gs.queryItems(Palapeli::GameStorageAttributes() << new Palapeli::GameStorageTypeAttribute(Palapeli::GameStorageItem::SavedGame) << new Palapeli::GameStorageMetaAttribute(name));
	if (configs.count() == 0)
		return;
	gs.removeItem(configs.at(0));
	emit savegameDeleted(name);
}

void Palapeli::Manager::savegameWasCreated(const QString& name)
{
	emit savegameCreated(name);
}

bool Palapeli::Manager::ensurePersistence()
{
	if (p->m_state.isPersistent())
		return true; //nothing to do
	if (p->m_gameName.isEmpty())
	{
		//game has not yet been saved
		const int answer = KMessageBox::warningContinueCancel(p->m_window, i18n("This will discard the game you're currently playing."));
		return answer == KMessageBox::Continue;
	}
	else
	{
		//game has already been saved previously - ask to save under same name again
		const int answer = KMessageBox::warningYesNoCancel(p->m_window, i18n("Do you want to save the changes you made in this game?"));
		switch (answer)
		{
			case KMessageBox::Yes:
				saveGame(p->m_gameName);
				return true;
			case KMessageBox::No:
				return true;
			default: //case KMessageBox::Cancel:
				return false;
		}
	}
}

//END Palapeli::Manager

#include "manager.moc"

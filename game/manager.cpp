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
#include "mainwindow.h"
#include "minimap.h"
#include "part.h"
#include "piece.h"
#include "piecerelation.h"
#include "preview.h"
#include "savegamemodel.h"
#include "view.h"

#include <QTimer>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KUrl>

namespace Palapeli
{

	struct ManagerPrivate
	{
		ManagerPrivate(Manager* manager);
		void init();
		~ManagerPrivate();

		bool canCreateGame(const KUrl& imageUrl, int patternIndex);
		bool canLoadGame(const QString& name);
		bool initGame();
		bool saveGame(const QString& name);

		Manager *m_manager;

		//current game
		QImage m_image;
		QUuid m_imageId;
		QUuid m_gameId;
		PatternConfiguration* m_patternConfiguration;
		int m_estimatePieceCount; //used during slice process to show progress
		//game and UI objects
		Minimap* m_minimap;
		QList<Part*> m_parts;
		QList<Piece*> m_pieces;
		Preview* m_preview;
		QList<PieceRelation> m_relations;
		SavegameModel* m_savegameModel;
		View* m_view;
		MainWindow* m_window;
	};

	namespace Strings
	{
		//strings in .psg files
		const QString GeneralGroupKey("Palapeli");
		const QString PatternKey("Pattern");
		const QString ImageFileKey("ImageSource");
		const QString PatternGroupKey("PatternArgs");
		const QString PiecesGroupKey("Pieces");
		const QString PositionKey("Position-%1");
	};

}

//BEGIN Palapeli::ManagerPrivate

Palapeli::ManagerPrivate::ManagerPrivate(Palapeli::Manager* manager)
	: m_manager(manager)
	, m_patternConfiguration(0)
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
		gameNames << item.metaData();
	qSort(gameNames.begin(), gameNames.end(), Palapeli::SavegameModel::lessThan);
	//initialize savegame model
	m_savegameModel = new Palapeli::SavegameModel(gameNames);
	QObject::connect(m_manager, SIGNAL(savegameCreated(const QString&)), m_savegameModel, SLOT(savegameCreated(const QString&)));
	QObject::connect(m_manager, SIGNAL(savegameDeleted(const QString&)), m_savegameModel, SLOT(savegameDeleted(const QString&)));
}

void Palapeli::ManagerPrivate::init()
{
	//The Manager needs a valid pointer to this ManagerPrivate instance before the following objects can be initialized.
	m_minimap = new Palapeli::Minimap;
	m_view = new Palapeli::View;
	m_window = new Palapeli::MainWindow;
	//main window is deleted by Palapeli::ManagerPrivate::~ManagerPrivate because there are widely-spread references to m_view, and m_view will be deleted by m_window
	m_window->setAttribute(Qt::WA_DeleteOnClose, false);
}

Palapeli::ManagerPrivate::~ManagerPrivate()
{
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
		if (patternConfig->property("patternName").toString() == patternName)
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
	m_patternConfiguration = patternConfiguration;
	KConfigGroup patternGroup(&config, Palapeli::Strings::PatternGroupKey);
	m_patternConfiguration->readArguments(&patternGroup);
	return true;
}

bool Palapeli::ManagerPrivate::initGame()
{
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
	m_estimatePieceCount = pattern->estimatePieceCount();
	if (!pieceBasePositions.isEmpty())
		pattern->loadPiecePositions(pieceBasePositions);
	QObject::connect(pattern, SIGNAL(pieceGenerated(const QImage&, const QRectF&, const QPointF&)),
		m_manager, SLOT(addPiece(const QImage&, const QRectF&, const QPointF&)));
	QObject::connect(pattern, SIGNAL(allPiecesGenerated()),
		m_manager, SLOT(endAddPiece()));
	QObject::connect(pattern, SIGNAL(relationGenerated(int, int, const QPointF&)),
		m_manager, SLOT(addRelation(int, int, const QPointF&)));
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
	//write pattern name and configuration
	QString patternName = m_patternConfiguration->property("patternName").toString();
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
	return true;
}

//END Palapeli::ManagerPrivate

//BEGIN Palapeli::Manager

Palapeli::Manager::Manager()
	: QObject()
	, p(new Palapeli::ManagerPrivate(this))
{
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
		p->m_window->reportProgress(0, realPieceCount, maxPieceCount, i18np("1 piece generated", "%1 pieces generated", realPieceCount));
	}
}

void Palapeli::Manager::endAddPiece()
{
	//keep application responsive
	int maxPieceCount = qMax(p->m_estimatePieceCount, p->m_pieces.count());
	p->m_window->reportProgress(0, maxPieceCount + 1, maxPieceCount + 1, i18n("Finding neighbors"));
}

void Palapeli::Manager::addRelation(int piece1Id, int piece2Id, const QPointF& positionDifference)
{
	p->m_relations << Palapeli::PieceRelation(p->m_pieces[piece1Id], p->m_pieces[piece2Id], positionDifference);
}

void Palapeli::Manager::removePart(Part* part)
{
	p->m_parts.removeAll(part);
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
		updateGraphics();
}

//game instances

void Palapeli::Manager::createGame(const KUrl& url, int patternIndex)
{
	if (!p->canCreateGame(url, patternIndex))
		return;
	emit interactionModeChanged(false);
	emit gameNameChanged(QString());
	p->initGame(); //partially asynchronous; slot finishGameLoading() will be called when finished
}

void Palapeli::Manager::loadGame(const QString& name)
{
	if (!p->canLoadGame(name))
		return;
	emit interactionModeChanged(false);
	emit gameNameChanged(name);
	p->initGame(); //partially asynchronous; slot finishGameLoading() will be called when finished
}

void Palapeli::Manager::finishGameLoading()
{
	//restore relations
	searchConnections();
	//propagate changes
	p->m_window->reportProgress(0, 1, 1, i18n("Game loaded."));
	QTimer::singleShot(1000, p->m_window, SLOT(flushProgress()));
	updateGraphics();
	p->m_view->useScene(true);
	emit interactionModeChanged(true);
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
	emit interactionModeChanged(false);
	p->saveGame(name);
	emit gameNameChanged(name);
	emit savegameCreated(name);
	emit interactionModeChanged(true);
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

void Palapeli::Manager::savegameWasDeleted(const QString& name)
{
	emit savegameDeleted(name);
}

//END Palapeli::Manager

#include "manager.moc"

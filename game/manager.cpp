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
#include "library/library.h"
#include "library/librarybase.h"
#include "library/puzzleinfo.h"
#include "mainwindow.h"
#include "minimap.h"
#include "part.h"
#include "piece.h"
#include "piecerelation.h"
#include "preview.h"
#include "settings.h"
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

		bool canLoadGame(Palapeli::PuzzleInfo* info);
		bool initGame(); //TODO: to be merged with canLoadGame eventually
		void saveGame(); //intended as autosave after every action

		Manager* m_manager;
		//current game
		Palapeli::PuzzleInfo m_puzzleInfo;
		PatternConfiguration* m_patternConfiguration;
		int m_estimatePieceCount; //used during slice process to show progress - TODO: necessary?
		//game and UI objects
		Library* m_library;
		Minimap* m_minimap;
		QList<Part*> m_parts;
		QList<Piece*> m_pieces;
		Preview* m_preview;
		QList<PieceRelation> m_relations;
		View* m_view;
		MainWindow* m_window;
	};

	namespace Strings
	{
		//strings in .desktop and .cfg files
		const QString GeneralGroupKey("X-Palapeli");
		const QString PatternKey("Pattern");
		const QString PatternGroupKey("X-PatternArgs");
		const QString PiecesGroupKey("X-Pieces");
		const QString PositionKey("Position-%1");
		const QString RelationsGroupKey("X-Relations");
	}

}

//BEGIN Palapeli::ManagerPrivate

Palapeli::ManagerPrivate::ManagerPrivate(Palapeli::Manager* manager)
	: m_manager(manager)
	, m_puzzleInfo(QString())
	, m_patternConfiguration(0)
	, m_library(new Palapeli::Library(new Palapeli::LibraryStandardBase))
	, m_minimap(0)
	, m_preview(new Palapeli::Preview)
	, m_view(0)
	, m_window(0)
{
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
	delete m_window; //the view is deleted here
}

bool Palapeli::ManagerPrivate::canLoadGame(Palapeli::PuzzleInfo* info)
{
	//check validity of input
	if (info->image.isNull())
		return false;
	if (info->identifier.isEmpty())
		return false;
	//do not load if the selected game is already loaded
	if (info->identifier == m_puzzleInfo.identifier)
		return false;
	//find configuration for this puzzle
	const QString mainConfigPath = m_library->base()->findFile(info->identifier, Palapeli::LibraryBase::MainConfigFile);
	KConfig mainConfig(mainConfigPath);
	//find pattern configuration
	KConfigGroup generalGroup(&mainConfig, Palapeli::Strings::GeneralGroupKey);
	const QString patternName = generalGroup.readEntry(Palapeli::Strings::PatternKey, QString());
	if (patternName.isEmpty())
		return false;
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
	//everything okay - save values for new game
	m_patternConfiguration = patternConfiguration;
	m_puzzleInfo = *info;
	return true;
}

bool Palapeli::ManagerPrivate::initGame()
{
	m_window->flushPuzzleProgress();
	//find state configuration file (returns a valid path to a non-existent file if no state config is available yet)
	const QString stateConfigPath = m_library->base()->findFile(m_puzzleInfo.identifier, Palapeli::LibraryBase::StateConfigFile);
	//read piece positions
	QList<QPointF> pieceBasePositions;
	KConfig stateConfig(stateConfigPath);
	KConfigGroup piecesGroup(&stateConfig, Palapeli::Strings::PiecesGroupKey);
	for (int i = 0; piecesGroup.hasKey(Palapeli::Strings::PositionKey.arg(i)); ++i)
		pieceBasePositions << piecesGroup.readEntry(Palapeli::Strings::PositionKey.arg(i), QPointF());
	//flush all variables
	foreach (Palapeli::Part* part, m_parts)
		delete part; //also deletes pieces
	m_parts.clear();
	m_pieces.clear();
	m_relations.clear();
	//configure scene and preview
	m_view->useScene(false); //disable the scene until the pieces have been built
	QRectF sceneRect(QPointF(0.0, 0.0), Settings::sceneSizeFactor() * m_puzzleInfo.image.size());
	m_view->realScene()->setSceneRect(sceneRect);
	m_preview->setImage(m_puzzleInfo.image);
	//instantiate a pattern
	Palapeli::Pattern* pattern = m_patternConfiguration->createPattern();
	m_estimatePieceCount = 0; //by now, we do not know how much pieces to await
	if (!pieceBasePositions.isEmpty())
		pattern->loadPiecePositions(pieceBasePositions);
	pattern->setSceneSizeFactor(Settings::sceneSizeFactor());
	QObject::connect(pattern, SIGNAL(estimatePieceCountAvailable(int)), m_manager, SLOT(estimatePieceCount(int)));
	QObject::connect(pattern, SIGNAL(pieceGenerated(const QImage&, const QRectF&, const QPointF&)),
		m_manager, SLOT(addPiece(const QImage&, const QRectF&, const QPointF&)));
	QObject::connect(pattern, SIGNAL(pieceGenerated(const QImage&, const QImage&, const QRectF&, const QPointF&)),
		m_manager, SLOT(addPiece(const QImage&, const QImage&, const QRectF&, const QPointF&)));
	QObject::connect(pattern, SIGNAL(allPiecesGenerated()),
		m_manager, SLOT(endAddPiece()));
	QObject::connect(pattern, SIGNAL(relationGenerated(int, int)),
		m_manager, SLOT(addRelation(int, int)));
	//create pieces, parts, relations (in another thread)
	Palapeli::PatternExecutor* patternExec = new Palapeli::PatternExecutor(pattern);
	patternExec->setImage(m_puzzleInfo.image);
	QObject::connect(patternExec, SIGNAL(finished()), m_manager, SLOT(finishGameLoading()));
	patternExec->start();
	patternExec->wait();
	return true;
}

void Palapeli::ManagerPrivate::saveGame()
{
	//TODO: should piece positions really be saved - config option?
	//TODO: relations are not yet restored because this is only necessary when piece positions are not saved
	//TODO: optimize by only syncing changes
	//open state config
	const QString stateConfigPath = m_library->base()->findFile(m_puzzleInfo.identifier, Palapeli::LibraryBase::StateConfigFile);
	KConfig stateConfig(stateConfigPath);
	//write piece positions
	KConfigGroup piecesGroup(&stateConfig, Palapeli::Strings::PiecesGroupKey);
	for (int i = 0; i < m_pieces.count(); ++i)
		piecesGroup.writeEntry(Palapeli::Strings::PositionKey.arg(i), m_pieces[i]->part()->basePosition());
	//write relation states
	KConfigGroup relationsGroup(&stateConfig, Palapeli::Strings::RelationsGroupKey);
	for (int i = 0; i < m_relations.count(); ++i)
		relationsGroup.writeEntry(QString::number(i), m_relations[i].combined());
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
	static Palapeli::Manager* theOneAndOnly = new Palapeli::Manager;
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

Palapeli::Library* Palapeli::Manager::library() const
{
	return p->m_library;
}

Palapeli::Minimap* Palapeli::Manager::minimap() const
{
	return p->m_minimap;
}

Palapeli::Preview* Palapeli::Manager::preview() const
{
	return p->m_preview;
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
	addPiece(piece, sceneBasePosition);
}

void Palapeli::Manager::addPiece(const QImage& baseImage, const QImage& mask, const QRectF& positionInImage, const QPointF& sceneBasePosition)
{
	Palapeli::Piece* piece = Palapeli::Piece::fromPixmapPair(QPixmap::fromImage(baseImage), QPixmap::fromImage(mask), positionInImage);
	addPiece(piece, sceneBasePosition);
}

void Palapeli::Manager::addPiece(Palapeli::Piece* piece, const QPointF& sceneBasePosition)
{
	p->m_pieces << piece;
	p->m_parts << new Palapeli::Part(piece);
	piece->part()->setBasePosition(sceneBasePosition);
	//keep application responsive
	//TODO: Does this have to be so complex? (If not, p->m_estimatePieceCount and the corresponding lib functions could be eliminated.)
	int realPieceCount = p->m_pieces.count();
	int updateStep = qMax(p->m_estimatePieceCount / 15, 5);
	if ((realPieceCount + updateStep / 2) % updateStep == 0) //do not redraw every time; this slows down the creation massively
	{
		p->m_window->reportPuzzleProgress(0, 0, i18np("1 piece generated", "%1 pieces generated", realPieceCount));
	}
}

void Palapeli::Manager::endAddPiece()
{
	//keep application responsive
	p->m_window->reportPuzzleProgress(0, 0, i18n("Finding neighbors"));
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
	p->saveGame();
}

void Palapeli::Manager::searchConnections()
{
	bool combinedSomething = false;
	foreach (const Palapeli::PieceRelation& rel, p->m_relations)
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

void Palapeli::Manager::loadGame(Palapeli::PuzzleInfo* info)
{
	if (!p->canLoadGame(info))
		return;
	emit interactionModeChanged(false);
	p->initGame(); //partially asynchronous; slot finishGameLoading() will be called when finished
}

void Palapeli::Manager::finishGameLoading()
{
	//restore relations
	searchConnections();
	//ensure that all pieces are inside the sceneRect (useful if user has changed the scene size after saving this game)
	const QRectF sceneRect = p->m_view->realScene()->sceneRect();
	foreach (Palapeli::Piece* piece, p->m_pieces)
	{
		const QRectF boundingRect = piece->sceneBoundingRect();
		if (!boundingRect.contains(sceneRect))
			piece->part()->move(piece->part()->basePosition()); //let's the part re-apply all movement constraints
	}
	//propagate changes
	updateGraphics();
	p->m_view->useScene(true);
	p->m_window->reportPuzzleProgress(p->m_pieces.count(), p->m_parts.count());
	emit interactionModeChanged(true);
	emit gameNameChanged(p->m_puzzleInfo.name);
}

//END Palapeli::Manager

#include "manager.moc"

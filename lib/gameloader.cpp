/***************************************************************************
 *   Copyright 2008-2009 Stefan Majewsky <majewsky@gmx.net>
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

#include "gameloader.h"
#include "gameloader_p.h"
#include "core/piece.h"
#include "core/piecerelation.h"
#include "core/view.h"
#include "library/library.h"
#include "library/librarybase.h"
#include "patterns/pattern.h"
#include "patterns/pattern-configuration.h"
#include "patterns/pattern-executor.h"
#include "patterns/pattern-trader.h"
#include "settings.h"

#include <QGraphicsScene>
#include <KConfig>
#include <KConfigGroup>

namespace Palapeli
{

	namespace Strings
	{
		//strings in .cfg files
		const QString PiecesGroupKey("X-Pieces");
		const QString PositionKey("Position-%1");
		const QString RelationsGroupKey("X-Relations");
	}

}

//BEGIN Palapeli::GameLoaderPrivate

Palapeli::GameLoaderPrivate::GameLoaderPrivate(Palapeli::Engine* engine, const Palapeli::PuzzleInfo* info, bool takeLibraryOwnership, Palapeli::GameLoader* parent)
	: m_isValid(true) //contains the "return value" of this constructor which can later be read through Palapeli::GameLoader::isValid
	, m_libraryOwnership(takeLibraryOwnership)
	, m_info(QString(), 0)
	, m_engine(engine)
{
	m_info = *info;
	//check validity of input
	if (info->image.isNull())
	{
		m_isValid = false;
		return;
	}
	if (info->identifier.isEmpty())
	{
		m_isValid = false;
		return;
	}
	//find pattern for this puzzle
	Palapeli::PatternConfiguration* patternConfiguration = Palapeli::PatternTrader::self()->configurationFromName(info->patternName);
	if (patternConfiguration == 0)
	{
		//no pattern found
		m_isValid = false;
		return;
	}
	//find configuration files (second one returns a valid path to a non-existent file if no state config is available yet)
	Palapeli::Library* library = info->library;
	const QString mainConfigPath = library->base()->findFile(info->identifier, Palapeli::LibraryBase::MainConfigFile);
	const QString stateConfigPath = library->base()->findFile(info->identifier, Palapeli::LibraryBase::StateConfigFile);
	//read pattern configuration
	KConfig mainConfig(mainConfigPath);
	KConfigGroup palapeliGroup(&mainConfig, "X-Palapeli");
	patternConfiguration->readArguments(&palapeliGroup);
	//read piece positions
	QList<QPointF> pieceBasePositions;
	KConfig stateConfig(stateConfigPath);
	KConfigGroup piecesGroup(&stateConfig, Palapeli::Strings::PiecesGroupKey);
	for (int i = 0; piecesGroup.hasKey(Palapeli::Strings::PositionKey.arg(i)); ++i)
		pieceBasePositions << piecesGroup.readEntry(Palapeli::Strings::PositionKey.arg(i), QPointF());
	//configure engine
	m_engine->clear();
	m_engine->view()->useScene(false); //disable the scene until the pieces have been built
	QRectF sceneRect(QPointF(0.0, 0.0), Settings::sceneSizeFactor() * info->image.size());
	m_engine->view()->realScene()->setSceneRect(sceneRect);
	//instantiate a pattern
	Palapeli::Pattern* pattern = patternConfiguration->createPattern();
	if (!pieceBasePositions.isEmpty())
		pattern->loadPiecePositions(pieceBasePositions);
	pattern->setSceneSizeFactor(Settings::sceneSizeFactor());
	QObject::connect(pattern, SIGNAL(pieceGenerated(const QImage&, const QRectF&, const QPointF&)),
		this, SLOT(addPiece(const QImage&, const QRectF&, const QPointF&)));
	QObject::connect(pattern, SIGNAL(pieceGenerated(const QImage&, const QImage&, const QRectF&, const QPointF&)),
		this, SLOT(addPiece(const QImage&, const QImage&, const QRectF&, const QPointF&)));
	QObject::connect(pattern, SIGNAL(relationGenerated(int, int)),
		this, SLOT(addRelation(int, int)));
	//create pieces, parts, relations (in another thread)
	Palapeli::PatternExecutor* patternExec = new Palapeli::PatternExecutor(pattern);
	patternExec->setImage(info->image);
	QObject::connect(patternExec, SIGNAL(finished()), parent, SLOT(finishLoading()));
	patternExec->start();
}

void Palapeli::GameLoaderPrivate::addPiece(const QImage& baseImage, const QImage& mask, const QRectF& positionInImage, const QPointF& sceneBasePosition)
{
	Palapeli::Piece* piece = Palapeli::Piece::fromPixmapPair(QPixmap::fromImage(baseImage), QPixmap::fromImage(mask), positionInImage, m_engine);
	m_engine->addPiece(piece, sceneBasePosition);
}

void Palapeli::GameLoaderPrivate::addPiece(const QImage& image, const QRectF& positionInImage, const QPointF& sceneBasePosition)
{
	Palapeli::Piece* piece = new Palapeli::Piece(QPixmap::fromImage(image), positionInImage, m_engine);
	m_engine->addPiece(piece, sceneBasePosition);
}

void Palapeli::GameLoaderPrivate::addRelation(int piece1Id, int piece2Id)
{
	m_engine->addRelation(piece1Id, piece2Id);
}

//END Palapeli::GameLoaderPrivate

//BEGIN Palapeli::GameLoader

Palapeli::GameLoader::GameLoader(Palapeli::Engine* engine, const Palapeli::PuzzleInfo* info, bool takeLibraryOwnership)
	: p(new Palapeli::GameLoaderPrivate(engine, info, takeLibraryOwnership, this))
{
	QObject::connect(engine, SIGNAL(pieceMoved()), this, SLOT(save()));
}

Palapeli::GameLoader::~GameLoader()
{
	//I see no benefit in declaring a separate destructor for Palapeli::GameLoaderPrivate.
	if (p->m_libraryOwnership)
		delete p->m_info.library;
	delete p;
}

const Palapeli::PuzzleInfo* Palapeli::GameLoader::info() const
{
	return &p->m_info;
}

bool Palapeli::GameLoader::isValid() const
{
	return p->m_isValid;
}

void Palapeli::GameLoader::finishLoading()
{
	p->m_engine->searchConnections();
	p->m_engine->updateProgress();
	//ensure that all pieces are inside the sceneRect (useful if user has changed the scene size after saving this game)
	const QRectF sceneRect = p->m_engine->view()->realScene()->sceneRect();
	for (int i = 0; i < p->m_engine->pieceCount(); ++i)
	{
		Palapeli::Piece* piece = p->m_engine->pieceAt(i);
		const QRectF boundingRect = piece->sceneBoundingRect();
		if (!sceneRect.contains(boundingRect))
			piece->part()->setPosition(piece->part()->pos()); //lets the part re-apply all movement constraints
	}
	emit finished();
	p->m_engine->view()->useScene(true);
}

void Palapeli::GameLoader::save()
{
	//TODO: should piece positions really be saved - config option?; write only connected relations
	//TODO: relations are not yet restored because this is only necessary when piece positions are not saved
	//TODO: optimize by only syncing changes
	//open state config
	const QString stateConfigPath = p->m_info.library->base()->findFile(p->m_info.identifier, Palapeli::LibraryBase::StateConfigFile);
	KConfig stateConfig(stateConfigPath);
	//write piece positions
	KConfigGroup piecesGroup(&stateConfig, Palapeli::Strings::PiecesGroupKey);
	for (int i = 0; i < p->m_engine->pieceCount(); ++i)
		piecesGroup.writeEntry(Palapeli::Strings::PositionKey.arg(i), p->m_engine->pieceAt(i)->part()->pos());
	//write relation states
	KConfigGroup relationsGroup(&stateConfig, Palapeli::Strings::RelationsGroupKey);
	for (int i = 0; i < p->m_engine->relationCount(); ++i)
		relationsGroup.writeEntry(QString::number(i), p->m_engine->relationAt(i).combined());
}

//END Palapeli::GameLoader

#include "gameloader.moc"
#include "gameloader_p.moc"

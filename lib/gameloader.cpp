/***************************************************************************
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

#include "gameloader.h"
#include "gameloader_p.h"
#include "core/engine.h"
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

Palapeli::GameLoaderPrivate::GameLoaderPrivate(const Palapeli::PuzzleInfo* info, bool takeLibraryOwnership, Palapeli::GameLoader* parent)
	: m_isValid(true) //contains the "return value" of this constructor which can later be read through Palapeli::GameLoader::isValid
	, m_libraryOwnership(takeLibraryOwnership)
	, m_info(*info)
{
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
	ppEngine()->clear();
	ppEngine()->view()->useScene(false); //disable the scene until the pieces have been built
	QRectF sceneRect(QPointF(0.0, 0.0), Settings::sceneSizeFactor() * info->image.size());
	ppEngine()->view()->realScene()->setSceneRect(sceneRect);
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
	Palapeli::Piece* piece = Palapeli::Piece::fromPixmapPair(QPixmap::fromImage(baseImage), QPixmap::fromImage(mask), positionInImage);
	ppEngine()->addPiece(piece, sceneBasePosition);
}

void Palapeli::GameLoaderPrivate::addPiece(const QImage& image, const QRectF& positionInImage, const QPointF& sceneBasePosition)
{
	Palapeli::Piece* piece = new Palapeli::Piece(QPixmap::fromImage(image), positionInImage);
	ppEngine()->addPiece(piece, sceneBasePosition);
}

void Palapeli::GameLoaderPrivate::addRelation(int piece1Id, int piece2Id)
{
	ppEngine()->addRelation(piece1Id, piece2Id);
}

//END Palapeli::GameLoaderPrivate

//BEGIN Palapeli::GameLoader

Palapeli::GameLoader::GameLoader(const Palapeli::PuzzleInfo* info, bool takeLibraryOwnership)
	: p(new Palapeli::GameLoaderPrivate(info, takeLibraryOwnership, this))
{
	QObject::connect(ppEngine(), SIGNAL(pieceMoved()), this, SLOT(save()));
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
	ppEngine()->searchConnections();
	emit finished();
	ppEngine()->view()->useScene(true);
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
	Palapeli::Engine* engine = ppEngine();
	for (int i = 0; i < engine->pieceCount(); ++i)
		piecesGroup.writeEntry(Palapeli::Strings::PositionKey.arg(i), engine->pieceAt(i)->part()->basePosition());
	//write relation states
	KConfigGroup relationsGroup(&stateConfig, Palapeli::Strings::RelationsGroupKey);
	for (int i = 0; i < engine->relationCount(); ++i)
		relationsGroup.writeEntry(QString::number(i), engine->relationAt(i).combined());
}

//END Palapeli::GameLoader

#include "gameloader.moc"
#include "gameloader_p.moc"

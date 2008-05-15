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
#include "cleanupthread.h"
#include "mainwindow.h"
#include "minimap.h"
#include "part.h"
#include "pattern-abstract.h"
#include "pattern-rect.h"
#include "piece.h"
#include "piecerelation.h"
#include "preview.h"
#include "savegameview.h"
#include "strings.h"
#include "view.h"

#include <QDir>
#include <QFile>
#include <QFuture>
#include <QFutureWatcher>
#include <QImage>
#include <QRegExp>
#include <QUuid>
#include <QtConcurrentRun>
#include <KConfig>
#include <KConfigGroup>
#include <kio/netaccess.h>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardDirs>

namespace Palapeli
{

	struct ManagerPrivate
	{
		ManagerPrivate(Manager* manager);
		void init();
		~ManagerPrivate();

		bool loadImage(const KUrl& url);
		bool loadImage(const QString& imageFile);
		void startGameInternal(Pattern* pattern);

		Manager *m_manager;

		//saved games
		KConfigGroup m_gamesConfig;
		QStringList m_games;
		//cleanup thread
		QFuture<QStringList> m_invalidGames;
		QFutureWatcher<QStringList> m_waitForInvalidGamesWatcher;
		//current game
		QImage m_image;
		QString m_imageFile;
		//game and UI objects
		Minimap* m_minimap;
		QList<Part*> m_parts;
		Pattern* m_pattern;
		QList<Piece*> m_pieces;
		Preview* m_preview;
		QList<PieceRelation> m_relations;
		SavegameView* m_savegameView;
		View* m_view;
		MainWindow* m_window;

	};

}

Palapeli::ManagerPrivate::ManagerPrivate(Palapeli::Manager* manager)
	: m_manager(manager)
	, m_gamesConfig(KGlobal::config(), Palapeli::Strings::GamesGroupKey)
	, m_games(m_gamesConfig.readEntry(Palapeli::Strings::GamesListKey, QStringList()))
	, m_invalidGames(QtConcurrent::run(Palapeli::cleanSavegameDirectory, m_games))
	, m_waitForInvalidGamesWatcher()
	, m_minimap(0)
	, m_pattern(0)
	, m_preview(new Palapeli::Preview)
	, m_savegameView(0)
	, m_view(0)
	, m_window(0)
{
	QObject::connect(&m_waitForInvalidGamesWatcher, SIGNAL(finished()), m_manager, SLOT(removeInvalidGames()));
	m_waitForInvalidGamesWatcher.setFuture(m_invalidGames);
}

void Palapeli::ManagerPrivate::init()
{
	//The Manager needs a valid pointer to this ManagerPrivate instance before the following objects can be initialized.
	m_minimap = new Palapeli::Minimap(m_manager);
	m_savegameView = new Palapeli::SavegameView(m_manager);
	m_view = new Palapeli::View(m_manager);
	m_window = new Palapeli::MainWindow(m_manager);
	//main window is deleted by Palapeli::ManagerPrivate::~ManagerPrivate because there are widely-spread references to m_view, and m_view will be deleted by m_window
	m_window->setAttribute(Qt::WA_DeleteOnClose, false);
}

Palapeli::ManagerPrivate::~ManagerPrivate()
{
	delete m_minimap;
	foreach (Palapeli::Part* part, m_parts)
		delete part; //the pieces are deleted here
	delete m_pattern;
	delete m_preview;
	delete m_savegameView;
	delete m_window; //the view is deleted here
}

bool Palapeli::ManagerPrivate::loadImage(const KUrl& url)
{
	//get extension of selected file
	static QRegExp extensionExtractor("(\\..*)$");
	QString extension = url.fileName();
	if (extensionExtractor.indexIn(extension, 0) >= 0)
		extension = extensionExtractor.cap(1);
	//generate a hopefully unique filename
	QUuid imageKey = QUuid::createUuid();
	QString imageFile = imageKey.toString().remove('{').remove('}') + extension;
	QString localFile = Palapeli::Strings::dataPath(imageFile);
	//download file
	if (url.isLocalFile())
	{
		QFile sourceFile(url.path());
		if (!sourceFile.copy(localFile))
		{
			KMessageBox::error(m_window, i18n("File could not be copied to the local image storage."));
			return false;
		}
	}
	else
	{
		if (!KIO::NetAccess::download(url, localFile, m_window))
		{
			KMessageBox::error(m_window, KIO::NetAccess::lastErrorString());
			return false;
		}
	}
	if (!m_image.load(localFile))
	{
		KMessageBox::error(m_window, i18n("File seems not to be an image file."));
		return false;
	}
	m_imageFile = imageFile;
	return true;
}

bool Palapeli::ManagerPrivate::loadImage(const QString& imageFile)
{
	QString localFile = Palapeli::Strings::dataPath(imageFile);
	if (!m_image.load(localFile))
	{
		KMessageBox::error(m_window, i18n("Image file is missing or corrupted."));
		return false;
	}
	m_imageFile = imageFile;
	return true;
}

void Palapeli::ManagerPrivate::startGameInternal(Palapeli::Pattern* pattern)
{
	//ATTENTION: This function assumes that the new image has been loaded into m_image.
	//flush all variables
	foreach (Palapeli::Part* part, m_parts)
		delete part; //also deletes pieces
	m_parts.clear();
	m_pieces.clear();
	m_relations.clear();
	//configure scene and preview
	m_view->scene()->setSceneRect(0, 0, 2 * m_image.width(), 2 * m_image.height());
	m_preview->setImage(m_image);
	//create pieces and parts
	delete m_pattern;
	m_pattern = pattern;
	m_pieces = m_pattern->slice(m_image);
}

//Palapeli::Manager class

Palapeli::Manager::Manager()
	: QObject()
	, p(new Palapeli::ManagerPrivate(this))
{
	p->init();
	//propagate list of save games to clients
	foreach (QString name, p->m_games)
		emit savegameCreated(name);
}

Palapeli::Manager::~Manager()
{
}

//properties

Palapeli::Minimap* Palapeli::Manager::minimap() const
{
	return p->m_minimap;
}

QListIterator<Palapeli::Part*> Palapeli::Manager::parts() const
{
	return QListIterator<Palapeli::Part*>(p->m_parts);
}

Palapeli::Pattern* Palapeli::Manager::pattern() const
{
	return p->m_pattern;
}

QListIterator<Palapeli::Piece*> Palapeli::Manager::pieces() const
{
	return QListIterator<Palapeli::Piece*>(p->m_pieces);
}

Palapeli::Preview* Palapeli::Manager::preview() const
{
	return p->m_preview;
}

QListIterator<Palapeli::PieceRelation> Palapeli::Manager::relations() const
{
	return QListIterator<Palapeli::PieceRelation>(p->m_relations);
}

Palapeli::SavegameView* Palapeli::Manager::savegameView() const
{
	return p->m_savegameView;
}

Palapeli::View* Palapeli::Manager::view() const
{
	return p->m_view;
}

Palapeli::MainWindow* Palapeli::Manager::window() const
{
	return p->m_window;
}

void Palapeli::Manager::updateMinimap()
{
	p->m_minimap->update();
}

//gameplay

void Palapeli::Manager::addRelation(Palapeli::Piece* piece1, Palapeli::Piece* piece2, const QPointF& positionDifference)
{
	p->m_relations << Palapeli::PieceRelation(piece1, piece2, positionDifference);
}

void Palapeli::Manager::searchConnections()
{
	static const qreal maxInaccuracyFactor = 0.1;
	bool combinedSomething = false;
	foreach (Palapeli::PieceRelation rel, p->m_relations)
	{
		if (rel.piece1()->part() == rel.piece2()->part()) //already combined
			continue;
		const qreal xMaxInaccuracy = maxInaccuracyFactor * rel.piece1()->size().width();
		const qreal yMaxInaccuracy = maxInaccuracyFactor * rel.piece1()->size().height();
		const QPointF posDiff = rel.piece2()->pos() - rel.piece1()->pos();
		const QPointF inaccuracy = posDiff - rel.positionDifference();
		if (qAbs(inaccuracy.x()) <= xMaxInaccuracy && qAbs(inaccuracy.y()) <= yMaxInaccuracy)
		{
			combine(rel.piece1()->part(), rel.piece2()->part());
			combinedSomething = true;
		}
	}
	if (combinedSomething)
		updateMinimap();
}

void Palapeli::Manager::combine(Palapeli::Part* part1, Palapeli::Part* part2)
{
	while (part2->m_pieces.count() > 0)
	{
		Palapeli::Piece* piece = part2->m_pieces.takeFirst();
		part2->remove(piece);
		part1->add(piece);
	}
	p->m_parts.removeAll(part2);
	part1->update();
	delete part2;
}

//game instances

void Palapeli::Manager::createGame(const KUrl& url, int xPieceCount, int yPieceCount)
{
	//load image
	if (!p->loadImage(url))
		return;
	//start game
	p->startGameInternal(new Palapeli::RectangularPattern(xPieceCount, yPieceCount, this));
	//random piece positions
	int sceneWidth = 2 * p->m_image.width(), sceneHeight = 2 * p->m_image.height();
	foreach (Palapeli::Piece* piece, p->m_pieces)
	{
		QRectF pieceRect = piece->sceneBoundingRect();
		piece->setPos(qrand() % (sceneWidth - (int) pieceRect.width()), qrand() % (sceneHeight - (int) pieceRect.height()));
		p->m_parts << new Palapeli::Part(piece, this);
	}
	//propagate changes
	updateMinimap();
	emit gameLoaded(QString());
}

void Palapeli::Manager::loadGame(const QString& name)
{
	if (!p->m_games.contains(name))
		return;
	//check if savegame exists
	const QString configFileName = Palapeli::Strings::configPath(name);
	if (configFileName.isEmpty())
		return;
	//load configuration and image
	KConfig config(configFileName);
	KConfigGroup generalGroup(&config, Palapeli::Strings::GeneralGroupKey);
	const QString imageFileName = generalGroup.readEntry(Palapeli::Strings::ImageFileKey, QString());
	if (!p->loadImage(imageFileName))
		return;
	//start game
	//TODO: Support multiple types of patterns. There is only "rectangular" at the moment, so I don't care about the name specified in "General/Pattern" at all.
	KConfigGroup patternGroup(&config, Palapeli::Strings::PatternGroupKey);
	p->startGameInternal(new Palapeli::RectangularPattern(&patternGroup, this));
	//restore piece positions and connections
	KConfigGroup piecesGroup(&config, Palapeli::Strings::PiecesGroupKey);
	for (int i = 0; i < p->m_pieces.count(); ++i)
	{
		Palapeli::Piece* piece = p->m_pieces.at(i);
		piece->setPos(piecesGroup.readEntry(Palapeli::Strings::PositionKey.arg(i), QPointF()));
		p->m_parts << new Palapeli::Part(piece, this);
	}
	searchConnections();
	//propagate changes
	updateMinimap();
	emit gameLoaded(name);
}

bool Palapeli::Manager::saveGame(const QString& name)
{
	if (!p->m_pattern || p->m_pieces.empty())
		return false;
	//FIXME: It is likely that more characters have to be excluded because of Windows. Can this be accomplished in a more elegant way?
	if (name.contains(QChar('/')) || name.contains(QChar('\\')))
	{
		KMessageBox::error(window(), i18n("Please choose a name that does not contain slashes."));
		return false;
	}
	if (name.startsWith(QLatin1String("__palapeli"), Qt::CaseInsensitive)) //case insensitivity because of Windows support
	{
		KMessageBox::error(window(), i18n("Please choose another name. Names starting with \"__palapeli\" are reserved for internal use."));
		return false;
	}
	//open config file and write general information
	KConfig config(Palapeli::Strings::configPath(name));
	KConfigGroup generalGroup(&config, Palapeli::Strings::GeneralGroupKey);
	generalGroup.writeEntry(Palapeli::Strings::PatternKey, p->m_pattern->name());
	generalGroup.writeEntry(Palapeli::Strings::ImageFileKey, p->m_imageFile);
	//pattern arguments
	KConfigGroup patternGroup(&config, Palapeli::Strings::PatternGroupKey);
	p->m_pattern->writeArguments(&patternGroup);
	//piece positions
	KConfigGroup pieceGroup(&config, Palapeli::Strings::PiecesGroupKey);
	for (int i = 0; i < p->m_pieces.count(); ++i)
	{
		Palapeli::Piece* piece = p->m_pieces.at(i);
		pieceGroup.writeEntry(Palapeli::Strings::PositionKey.arg(i), QVariant(piece->pos()));
	}
	//save information and image
	config.sync();
	//insert game into savegame list
	if (!p->m_games.contains(name))
	{
		p->m_games << name;
		p->m_gamesConfig.writeEntry(Palapeli::Strings::GamesListKey, p->m_games);
		p->m_gamesConfig.sync();
	}
	emit savegameCreated(name);
	return true;
}

void Palapeli::Manager::removeInvalidGames()
{
	p->m_invalidGames.waitForFinished(); //just to be sure that the result is really available
	foreach (QString game, p->m_invalidGames.result())
	{
		p->m_games.removeAll(game);
		emit savegameDeleted(game);
	}
	p->m_gamesConfig.writeEntry(Palapeli::Strings::GamesListKey, p->m_games);
	p->m_gamesConfig.sync();
}

void Palapeli::Manager::deleteGame(const QString& name)
{
	if (!p->m_games.contains(name))
		return;
	//remove game in savegame list
	p->m_games.removeAll(name);
	p->m_gamesConfig.writeEntry(Palapeli::Strings::GamesListKey, p->m_games);
	p->m_gamesConfig.sync();
	emit savegameDeleted(name);
	//Annotation: The config and image file will be deleted when the cleanup thread runs the next time. This is normally when Palapeli is started the next time.
}

#include "manager.moc"

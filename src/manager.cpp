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
#include "mainwindow.h"
#include "minimap.h"
#include "part.h"
#include "pattern-abstract.h"
#include "pattern-rect.h"
#include "piece.h"
#include "piecerelation.h"
#include "preview.h"
#include "savegameview.h"
#include "view.h"

#include <QDir>
#include <QFile>
#include <QImage>
#include <KComponentData>
#include <KConfig>
#include <KConfigGroup>
#include <kio/netaccess.h>
#include <KMessageBox>
#include <KStandardDirs>

//savegame storage strings
const QString saveGameDir("savegames");
const QString configPath("savegames/%1.psg");
const QString imagePath("savegames/%1.png");
//strings in .psg files
const QString generalGroupKey("Palapeli");
const QString patternKey("Pattern");
const QString imageKey("ImageSource");
const QString patternGroupKey("PatternArgs");
const QString piecesGroupKey("Pieces");
const QString positionKey("Position-%1");
//strings in palapelirc
const QString gamesGroupKey("Saved Games");
const QString gamesListKey("Names");

Palapeli::Manager::Manager()
	: QObject()
	, m_gamesConfig(new KConfigGroup(KGlobal::config(), gamesGroupKey))
	, m_games(m_gamesConfig->readEntry(gamesListKey, QStringList()))
	, m_image()
	, m_minimap(new Palapeli::Minimap(this))
	, m_pattern(0)
	, m_preview(new Palapeli::Preview)
	, m_savegameView(new Palapeli::SavegameView(this))
	, m_view(new Palapeli::View(this))
	, m_window(new Palapeli::MainWindow(this))
{
	//main window is managed by this class because there are widely-spread references to m_view, and m_view will be deleted by m_window
	m_window->setAttribute(Qt::WA_DeleteOnClose, false);
	//propagate list of save games to clients
	foreach (QString name, m_games)
		emit savegameCreated(name);
}

Palapeli::Manager::~Manager()
{
	delete m_minimap;
	foreach (Palapeli::Part* part, m_parts)
		delete part; //the pieces are deleted here
	delete m_pattern;
	delete m_preview;
	delete m_savegameView;
	delete m_window; //the view is deleted here
}

//properties

Palapeli::Minimap* Palapeli::Manager::minimap() const
{
	return m_minimap;
}

QListIterator<Palapeli::Part*> Palapeli::Manager::parts() const
{
	return QListIterator<Palapeli::Part*>(m_parts);
}

Palapeli::Pattern* Palapeli::Manager::pattern() const
{
	return m_pattern;
}

QListIterator<Palapeli::Piece*> Palapeli::Manager::pieces() const
{
	return QListIterator<Palapeli::Piece*>(m_pieces);
}

Palapeli::Preview* Palapeli::Manager::preview() const
{
	return m_preview;
}

QListIterator<Palapeli::PieceRelation> Palapeli::Manager::relations() const
{
	return QListIterator<Palapeli::PieceRelation>(m_relations);
}

Palapeli::SavegameView* Palapeli::Manager::savegameView() const
{
	return m_savegameView;
}

Palapeli::View* Palapeli::Manager::view() const
{
	return m_view;
}

Palapeli::MainWindow* Palapeli::Manager::window() const
{
	return m_window;
}

void Palapeli::Manager::updateMinimap()
{
	m_minimap->update();
}

//gameplay

void Palapeli::Manager::addRelation(Palapeli::Piece* piece1, Palapeli::Piece* piece2, const QPointF& positionDifference)
{
	m_relations << Palapeli::PieceRelation(piece1, piece2, positionDifference);
}

void Palapeli::Manager::searchConnections()
{
	static const qreal maxInaccuracyFactor = 0.1;
	bool combinedSomething = false;
	foreach (Palapeli::PieceRelation rel, m_relations)
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
	m_parts.removeAll(part2);
	part1->update();
	delete part2;
}

//game instances

QString Palapeli::Manager::toLocalFile(const KUrl& url)
{
	QString file;
	if(KIO::NetAccess::download(url, file, m_window))
	{
		m_localFiles << file;
		return file;
	}
	else
	{
		KMessageBox::error(m_window, KIO::NetAccess::lastErrorString());
		return QString();
	}
}

void Palapeli::Manager::cleanupTempFiles()
{
	foreach (const QString& file, m_localFiles)
		KIO::NetAccess::removeTempFile(file);
}

void Palapeli::Manager::createGame(const KUrl& url, int xPieceCount, int yPieceCount)
{
	//load image
	QString imageFile = toLocalFile(url);
	if (imageFile.isEmpty())
		return;
	m_image.load(imageFile);
	cleanupTempFiles();
	int sceneWidth = 2 * m_image.width(), sceneHeight = 2 * m_image.height();
	//flush all variables
	foreach (Palapeli::Part* part, m_parts)
		delete part; //also deletes pieces
	m_parts.clear();
	m_pieces.clear();
	m_relations.clear();
	//configure scene and preview
	m_view->scene()->setSceneRect(0, 0, sceneWidth, sceneHeight);
	m_preview->setImage(m_image);
	//create pieces and parts
	delete m_pattern;
	m_pattern = new Palapeli::RectangularPattern(xPieceCount, yPieceCount, this);
	m_pieces = m_pattern->slice(m_image);
	foreach (Palapeli::Piece* piece, m_pieces)
	{
		QRectF pieceRect = piece->sceneBoundingRect();
		piece->setPos(qrand() % (sceneWidth - (int) pieceRect.width()), qrand() % (sceneHeight - (int) pieceRect.height()));
		m_parts << new Palapeli::Part(piece, this);
	}
	updateMinimap();
	emit gameLoaded(QString());
}

void Palapeli::Manager::loadGame(const QString& name)
{
	if (!m_games.contains(name))
		return;
	//check if savegame exists
	const QString configFileName = KStandardDirs::locate("appdata", configPath.arg(name));
	const QString imageFileName = KStandardDirs::locate("appdata", imagePath.arg(name));
	if (configFileName.isEmpty() || imageFileName.isEmpty())
		return;
	//load image and configuration
	KConfig config(configFileName);
	m_image.load(imageFileName);
	int sceneWidth = 2 * m_image.width(), sceneHeight = 2 * m_image.height();
	//flush all variables
	foreach (Palapeli::Part* part, m_parts)
		delete part; //also deletes pieces
	m_parts.clear();
	m_pieces.clear();
	m_relations.clear();
	//configure scene and preview
	m_view->scene()->setSceneRect(0, 0, sceneWidth, sceneHeight);
	m_preview->setImage(m_image);
	//create pieces and parts
	delete m_pattern;
	//TODO: Support multiple types of patterns. There is only "rectangular" at the moment, so I don't care about the name specified in "General/Pattern" at all.
	m_pattern = new Palapeli::RectangularPattern(config.entryMap(patternGroupKey), this);
	m_pieces = m_pattern->slice(m_image);
	KConfigGroup piecesGroup(&config, piecesGroupKey);
	for (int i = 0; i < m_pieces.count(); ++i)
	{
		Palapeli::Piece* piece = m_pieces.at(i);
		piece->setPos(piecesGroup.readEntry(positionKey.arg(i), QPointF()));
		m_parts << new Palapeli::Part(piece, this);
	}
	searchConnections(); //reconnect everything which was already connected
	updateMinimap();
	emit gameLoaded(name);
}

void Palapeli::Manager::saveGame(const QString& name)
{
	if (!m_pattern || m_pieces.empty())
		return;
	//open config file and write general information
	KConfig config(KStandardDirs::locateLocal("appdata", configPath.arg(name)));
	KConfigGroup generalGroup(&config, generalGroupKey);
	generalGroup.writeEntry(patternKey, m_pattern->name());
	//pattern arguments
	KConfigGroup patternGroup(&config, patternGroupKey);
	QMap<QString,QString> args = m_pattern->args();
	QMapIterator<QString,QString> iterArgs(args);
	while (iterArgs.hasNext())
	{
		iterArgs.next();
		patternGroup.writeEntry(iterArgs.key(), iterArgs.value());
	}
	//piece positions
	KConfigGroup pieceGroup(&config, piecesGroupKey);
for (int i = 0; i < m_pieces.count(); ++i)
	{
		Palapeli::Piece* piece = m_pieces.at(i);
		pieceGroup.writeEntry(positionKey.arg(i), QVariant(piece->pos()));
	}
	//save information and image
	config.sync();
	m_image.save(KStandardDirs::locateLocal("appdata", imagePath.arg(name)), "PNG"); //format should be lossless
	//insert game into savegame list
	if (!m_games.contains(name))
	{
		m_games << name;
		m_gamesConfig->writeEntry(gamesListKey, m_games);
		m_gamesConfig->sync();
	}
	emit savegameCreated(name);
}

void Palapeli::Manager::deleteGame(const QString& name)
{
	if (!m_games.contains(name))
		return;
	QFile(KStandardDirs::locateLocal("appdata", configPath.arg(name))).remove();
	QFile(KStandardDirs::locateLocal("appdata", imagePath.arg(name))).remove();
	//remove game in savegame list
	m_games.removeAll(name);
	m_gamesConfig->writeEntry(gamesListKey, m_games);
	m_gamesConfig->sync();
	emit savegameDeleted(name);
}

#include "manager.moc"

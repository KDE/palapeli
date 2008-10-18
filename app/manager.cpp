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
#include "../lib/core/engine.h"
#include "../lib/core/part.h"
#include "../lib/core/piece.h"
#include "../lib/core/piecerelation.h"
#include "../lib/core/view.h"
#include "../lib/library/library.h"
#include "../lib/library/librarybase.h"
#include "../lib/library/puzzleinfo.h"
#include "../lib/gameloader.h"
#include "actions/commonaction.h"
#include "actions/importaction.h"
#include "mainwindow.h"
#include "minimap.h"
#include "preview.h"

#include <KCmdLineArgs>
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

		Manager* m_manager;
		GameLoader* m_loader;
		//game and UI objects
		Minimap* m_minimap;
		Preview* m_preview;
		MainWindow* m_window;
	};

}

//BEGIN Palapeli::ManagerPrivate

Palapeli::ManagerPrivate::ManagerPrivate(Palapeli::Manager* manager)
	: m_manager(manager)
	, m_loader(0)
	, m_minimap(0)
	, m_preview(0)
	, m_window(0)
{
	//The GUI initialisation is deferred to a separate init() method because the Manager needs a valid pointer to this ManagerPrivate instance before some of the members of this instance can be initialized.
}

void Palapeli::ManagerPrivate::init()
{
	m_minimap = new Palapeli::Minimap;
	m_preview = new Palapeli::Preview;
	m_window = new Palapeli::MainWindow;
	//main window is deleted by Palapeli::ManagerPrivate::~ManagerPrivate because there are widely-spread references to the view, and the view will be deleted by m_window
	m_window->setAttribute(Qt::WA_DeleteOnClose, false);
	//make some connections
	QObject::connect(ppEngine(), SIGNAL(piecePositionChanged()), m_minimap, SLOT(update()));
	QObject::connect(ppEngine(), SIGNAL(relationsCombined()), m_manager, SLOT(updateProgress()));
	QObject::connect(ppEngine(), SIGNAL(viewportMoved()), m_minimap, SLOT(update()));
}

Palapeli::ManagerPrivate::~ManagerPrivate()
{
	delete m_minimap;
	delete m_preview;
	delete m_window; //attention: ppEngine()->view() might be deleted here
}

//END Palapeli::ManagerPrivate

//BEGIN Palapeli::Manager

Palapeli::Manager::Manager()
	: p(new Palapeli::ManagerPrivate(this))
{
}

Palapeli::Manager::~Manager()
{
}

Palapeli::Manager* Palapeli::Manager::self()
{
	static Palapeli::Manager theOneAndOnly;
	return &theOneAndOnly;
}

bool Palapeli::Manager::init()
{
	//initialize ManagerPrivate - This cannot be done automatically be done in the constructor: In this case, there would be another call to ppMgr() before theOneAndOnly in Palapeli::Manager::self is fully constructed, i.e. ppMgr() returns an uninitialized int.
	static bool initialized = false; //make sure this happens only once
	if (initialized)
		return true;
	initialized = true;
	//read arguments
	KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
	if (args->isSet("import"))
	{
		const QString importUrlString = args->getOption("import");
		const KUrl importUrl = KCmdLineArgs::makeURL(importUrlString.toUtf8());
		Palapeli::ImportDialog importDialog(importUrl);
		if (importDialog.isArchiveValid())
			importDialog.exec();
	}
	if (!args->isSet("gui"))
		return false; //no need to continue execution
	//Now, we're sure that we need the complete GUI.
	p->init();
	p->m_window->show();
	Palapeli::Actions::setDialogParent(p->m_window);
	Palapeli::Actions::setInfoGetter(&Palapeli::Manager::staticPuzzleInfo);
	//load game if requested
	if (args->count() != 0)
	{
		const KUrl playUrl = args->url(0);
		Palapeli::LibraryArchiveBase* playBase = new Palapeli::LibraryArchiveBase(playUrl);
		if (!playBase->findEntries().isEmpty())
		{
			Palapeli::Library* lib = new Palapeli::Library(playBase);
			Palapeli::PuzzleInfo* info = lib->infoForPuzzle(0); //0 = the first puzzle
			loadGame(info, false, true); //lib will be deleted automatically when the GameLoader is killed
		}
		else
		{
			KMessageBox::error(0, i18n("This puzzle archive is unreadable. It might be corrupted.")); //note: no window is being shown at the moment
			delete playBase;
		}
	}
	args->clear();
	return true;
}

//properties

const Palapeli::PuzzleInfo* Palapeli::Manager::puzzleInfo() const
{
	return p->m_loader ? p->m_loader->info() : 0;
}

const Palapeli::PuzzleInfo* Palapeli::Manager::staticPuzzleInfo()
{
	return ppMgr()->puzzleInfo();
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

//game instances

void Palapeli::Manager::loadGame(const Palapeli::PuzzleInfo* info, bool forceReload, bool takeLibraryOwnership)
{
	//do not load if the selected game is already loaded
	if (!forceReload && p->m_loader)
	{
		if (p->m_loader->info()->identifier == info->identifier)
			return;
	}
	//create loader - note that the old loader cannot be safely deleted before the new one has been created because the new one could (for forceReload == true) use the same PuzzleInfo object; the new loader has to be fully constructed to be sure that it has created its own puzzle info copy
	Palapeli::GameLoader* loader = new Palapeli::GameLoader(info, takeLibraryOwnership);
	if (!loader->isValid())
		return;
	delete p->m_loader; //note also that for the above reason you may not use info anymore; use loader->info() instead
	p->m_loader = loader;
	connect(p->m_loader, SIGNAL(finished()), this, SLOT(finishGameLoading()));
	//update GUI
	p->m_window->reportPuzzleProgress(0, 0, i18n("Loading puzzle..."));
	p->m_preview->setImage(loader->info()->image);
	emit interactionModeChanged(false);
}

void Palapeli::Manager::finishGameLoading()
{
	//ensure that all pieces are inside the sceneRect (useful if user has changed the scene size after saving this game)
	const QRectF sceneRect = ppEngine()->view()->realScene()->sceneRect();
	for (int i = 0; i < ppEngine()->pieceCount(); ++i)
	{
		Palapeli::Piece* piece = ppEngine()->pieceAt(i);
		const QRectF boundingRect = piece->sceneBoundingRect();
		if (!boundingRect.contains(sceneRect))
			piece->part()->move(piece->part()->basePosition()); //let's the part re-apply all movement constraints
	}
	//propagate changes
	p->m_minimap->update();
	p->m_window->reportPuzzleProgress(ppEngine()->pieceCount(), ppEngine()->partCount());
	emit interactionModeChanged(true);
	emit gameNameChanged(p->m_loader->info()->name);
}

void Palapeli::Manager::updateProgress()
{
	p->m_window->reportPuzzleProgress(ppEngine()->pieceCount(), ppEngine()->partCount());
}

//END Palapeli::Manager

#include "manager.moc"

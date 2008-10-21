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

#include "part.h"
#include "../app/main.h"
#include "../lib/actions/resetaction.h"
#include "../lib/core/engine.h"
#include "../lib/core/view.h"
#include "../lib/gameloader.h"
#include "../lib/library/library.h"
#include "../lib/library/librarybase.h"
#include "../lib/library/puzzleinfo.h"
#include "../lib/textprogressbar.h"

#include <KActionCollection>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KStatusBar>
#include <kparts/statusbarextension.h>

K_PLUGIN_FACTORY(PalapeliPartFactory, registerPlugin<Palapeli::KPart>();)
K_EXPORT_PLUGIN(PalapeliPartFactory(Palapeli::aboutData()))

Palapeli::KPart::KPart(QWidget* parentWidget, QObject* parent, const QVariantList& args)
	: KParts::ReadOnlyPart(parent)
	, m_statusBar(new KParts::StatusBarExtension(this))
	, m_statusBarInitialized(false)
	, m_engine(new Palapeli::Engine)
	, m_loader(0)
{
	Q_UNUSED(parentWidget)
	Q_UNUSED(args)
	//create component and load engine
	setComponentData(PalapeliPartFactory::componentData());
	m_engine->setDeleteViewInDestructor(false);
	setWidget(m_engine->view()); //FIXME: flicker appears if the view is shown before the loading starts
	//add actions and statusbar widgets
	/*KAction* action =*/ new Palapeli::ResetAction(actionCollection());
	//TODO: connect action and adjust behavior
	//load XMLGUI resource file
	setXMLFile("palapelipart.rc");
}

Palapeli::KPart::~KPart()
{
	closeUrl();
	delete m_engine;
	delete m_statusBar;
}

bool Palapeli::KPart::closeUrl()
{
	KParts::ReadOnlyPart::closeUrl();
	delete m_loader;
	m_loader = 0;
	return true;
}

bool Palapeli::KPart::load(const Palapeli::PuzzleInfo* info)
{
	Palapeli::PuzzleInfo newInfo = *info; //the instance "info" might get destroyed when destroying m_loader in closeUrl()
	closeUrl();
	m_loader = new Palapeli::GameLoader(m_engine, info, true);
	if (m_loader->isValid())
	{
		emit setWindowCaption(newInfo.name); //FIXME: Why am I not shown?
		return true;
	}
	else
		return false;
}

bool Palapeli::KPart::openFile()
{
	if (!m_statusBarInitialized)
	{
		//this cannot be done in the constructor because the status bar needs to be shown first
		m_statusBar->addStatusBarItem(m_engine->progressBar(), 1, true);
		m_statusBarInitialized = true;
	}
	Palapeli::LibraryBase* base = new Palapeli::LibraryArchiveBase(localFilePath());
	if (base->findEntries().isEmpty())
	{
		delete base;
		return false;
	}
	Palapeli::Library* library = new Palapeli::Library(base);
	base->setParent(library); //ensure that base is deleted
	Palapeli::PuzzleInfo* info = library->infoForPuzzle(0); //0 = the first puzzle
	return load(info);
}

#include "part.moc"

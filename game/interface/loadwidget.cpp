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

#include "loadwidget.h"
#include "interfacemanager.h"
#include "../manager.h"
#include "onscreenanimator.h"
#include "../savegamemodel.h"

#include <QListView>
#include <QTimer>
#include <KActionCollection>
#include <KLocalizedString>
#include <KStandardGuiItem>
#include <KStandardShortcut>

//BEGIN Palapeli::LoadWidget

Palapeli::LoadWidget* Palapeli::LoadWidget::create(Palapeli::AutoscalingItem* parent)
{
	QListView* view = new QListView;
	view->setModel(ppMgr()->savegameModel());
	view->setSelectionMode(QAbstractItemView::SingleSelection);
	return new Palapeli::LoadWidget(view, parent);
}

Palapeli::LoadWidget::LoadWidget(QListView* view, Palapeli::AutoscalingItem* parent)
	: Palapeli::OnScreenDialog(view, QList<KGuiItem>() << KStandardGuiItem::open() << KStandardGuiItem::del() << KStandardGuiItem::cancel(), i18n("Open a saved game"), parent)
	, m_view(view)
{
	connect(this, SIGNAL(buttonPressed(int)), this, SLOT(handleButton(int)));
	connect(m_view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(handleSelectionChange()));
	setButtonEnabled(0, false); //FIXME: does not work
	update();
}

void Palapeli::LoadWidget::handleButton(int id)
{
	if (id == 1) //the "Delete" button
	{
		//delete selected games
		foreach (const QModelIndex& item, m_view->selectionModel()->selectedIndexes())
			ppMgr()->deleteGame(ppMgr()->savegameModel()->data(item, Qt::DisplayRole).toString());
	}
	else
	{
		if (id == 0) //the "Load" button
			QTimer::singleShot(animator()->duration(), this, SLOT(load())); //start loading after this item has been hidden
		ppIMgr()->hide(Palapeli::InterfaceManager::LoadWidget);
	}
}

void Palapeli::LoadWidget::handleSelectionChange()
{
	setButtonEnabled(0, m_view->selectionModel()->hasSelection());
}

void Palapeli::LoadWidget::load()
{
	const QModelIndexList indexes = m_view->selectionModel()->selectedIndexes();
	if (indexes.isEmpty())
		return;
	const QString name = indexes[0].data(Qt::DisplayRole).toString();
	if (!name.isEmpty())
		ppMgr()->loadGame(name);
}

//END Palapeli::LoadWidget

//BEGIN Palapeli::LoadWidgetAction

Palapeli::LoadWidgetAction::LoadWidgetAction(QObject* parent)
	: KAction(KIcon("document-load"), i18n("&Open"), parent)
{
	setObjectName("palapeli_load");
	setShortcut(KStandardShortcut::shortcut(KStandardShortcut::Open));
	setToolTip(i18n("Open a saved game"));

	connect(this, SIGNAL(triggered(bool)), this, SLOT(trigger()));
	connect(ppMgr(), SIGNAL(savegameCreated(const QString&)), this, SLOT(gameCountChanged()));
	connect(ppMgr(), SIGNAL(savegameDeleted(const QString&)), this, SLOT(gameCountChanged()));
	gameCountChanged(); //adapt to initial state

	KActionCollection* collection = qobject_cast<KActionCollection*>(parent);
	if (collection)
		collection->addAction(objectName(), this);
}

void Palapeli::LoadWidgetAction::gameCountChanged()
{
	setEnabled(ppMgr()->savegameModel()->savegameCount() > 0);
}

void Palapeli::LoadWidgetAction::trigger()
{
	ppIMgr()->show(Palapeli::InterfaceManager::LoadWidget);
}

//END Palapeli::LoadWidgetAction

#include "loadwidget.moc"

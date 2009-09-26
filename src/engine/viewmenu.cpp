/***************************************************************************
 *   Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
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

#include "viewmenu.h"
#include "viewmenu-components.h"

#include <QFileInfo>
#include <QGraphicsScene>
#include <QWidgetAction>
#include <KLocalizedString>
#include <KStandardDirs>

Palapeli::ViewMenu::ViewMenu(QGraphicsScene* scene)
	: m_scene(scene)
{
	const QString selectedFileName = QString(); //TODO: read this from settings when kcfg stuff has been set up
	static const QString defaultFileName("background.svg");
	int selectedIndex = -1, defaultIndex = -1, currentIndex = -1;
	//fetch backgrounds, and create menu items
	QStringList backgroundFiles = KStandardDirs().findAllResources("data", "palapeli/backgrounds/*");
	QList<Palapeli::ViewMenuItem*> menuItems;
	foreach (const QString& path, backgroundFiles)
	{
		++currentIndex;
		//get file name and find selected or default backgrounds
		const QString fileName = QFileInfo(path).fileName();
		if (fileName == selectedFileName)
			selectedIndex = currentIndex;
		if (fileName == defaultFileName)
			defaultIndex = currentIndex;
		//create menu item for this brush
		Palapeli::ViewMenuItem* item = new Palapeli::ViewMenuItem(fileName);
		menuItems << item;
		connect(item, SIGNAL(startPreview(const QBrush&)), this, SLOT(startPreview(const QBrush&)));
		connect(item, SIGNAL(selected(const QString&, const QBrush&)), this, SLOT(selected(const QString&, const QBrush&)));
	}
	//select initial brush
	if (selectedIndex >= 0)
		m_currentItem = menuItems[selectedIndex];
	else
		m_currentItem = menuItems[qMax(defaultIndex, 0)]; //the qMax chooses the index 0 if defaultIndex == -1
	stopPreview(); //this method call actually loads the default brush into the scene
	//build container widget
	Palapeli::ViewMenuWidget* container = new Palapeli::ViewMenuWidget(menuItems);
	connect(container, SIGNAL(stopPreview()), this, SLOT(stopPreview()));
	//build menu
	addTitle(i18n("Select puzzle table texture"));
	QWidgetAction* act = new QWidgetAction(this);
	act->setDefaultWidget(container);
	addAction(act);
	addTitle(i18n("Hover to preview texture and click to select it."));
}

void Palapeli::ViewMenu::showAtCursorPosition()
{
	//ensure that menu item of current brush is under the mouse at the beginning
	const QPoint cursorTarget = m_currentItem->mapTo(this, m_currentItem->rect().center());
	popup(QCursor::pos() - cursorTarget);
	//Note that popup() ensures that the menu is completely visible on screen.
}

void Palapeli::ViewMenu::startPreview(const QBrush& brush)
{
	m_scene->setBackgroundBrush(brush);
}

void Palapeli::ViewMenu::stopPreview()
{
	m_scene->setBackgroundBrush(m_currentItem->brush());
}

void Palapeli::ViewMenu::selected(const QString& fileName, const QBrush& brush)
{
	hide();
	//change default background
	m_currentItem = qobject_cast<Palapeli::ViewMenuItem*>(sender());
	m_scene->setBackgroundBrush(brush);
	//TODO: save brush in config (cannot be implemented currently because kcfg stuff has not yet been set up)
}

#include "viewmenu.moc"

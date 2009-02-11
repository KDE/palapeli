/***************************************************************************
 *   Copyright (C) 2009 Stefan Majewsky <majewsky@gmx.net>
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
#include "view.h"
#include "settings.h"

#include <QBrush>
#include <QFileInfo>
#include <QGridLayout>
#include <QPushButton>
#include <QPixmap>
#include <QSvgRenderer>
#include <QWidget>
#include <QWidgetAction>
#include <KLocalizedString>
#include <KStandardDirs>

namespace Palapeli
{

	class ViewMenuItem : public QPushButton
	{
		private:
			QString m_fileName;
			QBrush m_pixmapBrush;
		public:
			ViewMenuItem(const QString& fileName, const QPixmap& pixmap);

			QString fileName() const { return m_fileName; }
			QBrush pixmapBrush() const { return m_pixmapBrush; }
	};

	class ViewMenuPrivate
	{
		public:
			ViewMenuPrivate(Palapeli::ViewMenu* parent);

			QWidget* m_container;
			QGridLayout* m_layout;
			QList<Palapeli::ViewMenuItem*> m_items;
			Palapeli::ViewMenuItem* m_currentItem;
	};

}

//BEGIN Palapeli::ViewMenuItem

static const int ViewMenuItemImageSize = 128;

Palapeli::ViewMenuItem::ViewMenuItem(const QString& fileName, const QPixmap& pixmap)
	: QPushButton(QIcon(pixmap.scaled(ViewMenuItemImageSize, ViewMenuItemImageSize, Qt::KeepAspectRatio)), QString())
	, m_fileName(fileName)
	, m_pixmapBrush(pixmap)
{
	setIconSize(QSize(ViewMenuItemImageSize, ViewMenuItemImageSize));
}

//END Palapeli::ViewMenuItem

//BEGIN Palapeli::ViewMenuPrivate

Palapeli::ViewMenuPrivate::ViewMenuPrivate(Palapeli::ViewMenu* parent)
	: m_container(new QWidget)
	, m_layout(new QGridLayout)
{
	static const QString backgroundFolder("palapeli/backgrounds/");
	const QString selectedFileName = Settings::viewBackground();
	static const QString defaultFileName("background.svg"); //see palapeli/app/pics/README.artists
	int selectedIndex = -1;
	int defaultIndex = -1;
	int currentIndex = -1;
	//fetch backgrounds
	QStringList backgroundFiles = KStandardDirs().findAllResources("data", backgroundFolder + "*", KStandardDirs::NoDuplicates);
	foreach (const QString& path, backgroundFiles)
	{
		++currentIndex;
		QPixmap pixmap;
		if (path.contains(".svg"))
		{
			QSvgRenderer renderer(path);
			pixmap = QPixmap(renderer.defaultSize());
			pixmap.fill(Qt::transparent);
			QPainter painter(&pixmap);
			renderer.render(&painter);
			painter.end();
		}
		else
			pixmap.load(path);
		//get file name and find selected or default backgrounds
		const QString fileName = QFileInfo(path).fileName();
		if (fileName == selectedFileName)
			selectedIndex = currentIndex;
		if (fileName == defaultFileName)
			defaultIndex = currentIndex;
		//create menu item for this brush
		Palapeli::ViewMenuItem* item = new Palapeli::ViewMenuItem(fileName, pixmap);
		m_items << item;
		QObject::connect(item, SIGNAL(clicked()), parent, SLOT(backgroundSelected()));
	}
	//select initial brush
	if (selectedIndex >= 0)
		m_currentItem = m_items[selectedIndex];
	else
		m_currentItem = m_items[qMax(defaultIndex, 0)]; //the qMax chooses the index 0 if defaultIndex == -1
	//build grid layout for menu items
	int x = -1, y = 0;
	const int columnCount = 4;
	foreach (Palapeli::ViewMenuItem* item, m_items)
	{
		x = (x + 1) % columnCount;
		if (x == 0)
			++y;
		m_layout->addWidget(item, x, y);
	}
	m_container->setLayout(m_layout);
}

//END Palapeli::ViewMenuPrivate

Palapeli::ViewMenu::ViewMenu(Palapeli::View* view)
	: KMenu(view)
	, p(new Palapeli::ViewMenuPrivate(this))
{
	addTitle(i18n("Select puzzle table texture"));
	QWidgetAction* act = new QWidgetAction(this);
	act->setDefaultWidget(p->m_container);
	addAction(act);
}

Palapeli::ViewMenu::~ViewMenu()
{
	delete p;
}

QBrush Palapeli::ViewMenu::currentBackground()
{
	return p->m_currentItem->pixmapBrush();
}

void Palapeli::ViewMenu::showAtCursorPosition()
{
	//TODO: menu item of current brush should be under the mouse
	popup(QCursor::pos());
}

void Palapeli::ViewMenu::backgroundSelected()
{
	Palapeli::ViewMenuItem* item = dynamic_cast<Palapeli::ViewMenuItem*>(sender());
	if (!item)
		return;
	hide();
	//change default background
	p->m_currentItem = item;
	emit backgroundSelected(item->pixmapBrush());
	//save brush in config
	Settings::setViewBackground(item->fileName());
	Settings::self()->writeConfig();
}

#include "viewmenu.moc"

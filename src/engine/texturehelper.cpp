/***************************************************************************
 *   Copyright 2009, 2010 Stefan Majewsky <majewsky@gmx.net>
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

#include "texturehelper.h"
#include "settings.h"

#include <QFileInfo>
#include <QGraphicsScene>
#include <QPainter>
#include <KGlobal>
#include <KLocalizedString>
#include <KStandardDirs>
#include <KSvgRenderer>

const QSize Palapeli::TextureHelper::DefaultThumbnailSize(32, 32);
const QSize Palapeli::TextureHelper::DefaultPixmapSize(128, 128);

QPixmap Palapeli::TextureHelper::render(const QString& fileName)
{
	const QString path = KStandardDirs::locate("appdata", "backgrounds/" + fileName);
	QPixmap pixmap;
	if (fileName.contains(".svg"))
	{
		KSvgRenderer renderer(path);
		pixmap = QPixmap(DefaultPixmapSize);
		pixmap.fill(Qt::transparent);
		QPainter painter(&pixmap);
		renderer.render(&painter);
		painter.end();
	}
	else
		pixmap.load(path);
	return pixmap;
}

Palapeli::TextureHelper::TextureHelper(QGraphicsScene* scene)
	: m_scene(scene)
	, m_currentIndex(-1)
{
	const QString selectedStyle = Settings::viewBackgroundStyle();
	const QColor selectedColor = Settings::viewBackgroundColor();
	const QString selectedFileName = Settings::viewBackground();
	int selectedIndex = 0, currentIndex = 0;
	//create menu item for solid color
	QPixmap colorThumbnail(DefaultThumbnailSize);
	colorThumbnail.fill(selectedColor);
	QStandardItem* colorItem = new QStandardItem;
	colorItem->setData(selectedColor, BrushRole);
	colorItem->setData("color", StyleRole);
	colorItem->setData(colorThumbnail, Qt::DecorationRole);
	colorItem->setData(i18nc("@item:inlistbox", "Single color"), Qt::DisplayRole);
	appendRow(colorItem);
	//fetch backgrounds, and create menu items
	const QStringList backgroundFiles = KGlobal::dirs()->findAllResources("appdata", "backgrounds/*");
	foreach (const QString& path, backgroundFiles)
	{
		++currentIndex;
		//get file name and find selected or default backgrounds
		const QString fileName = QFileInfo(path).fileName();
		if (fileName == selectedFileName && selectedStyle == "texture")
			selectedIndex = currentIndex;
		//create item for this brush
		const QPixmap pixmap = render(fileName);
		QStandardItem* item = new QStandardItem;
		item->setData(pixmap, BrushRole);
		item->setData("texture", StyleRole);
		item->setData(pixmap.scaled(DefaultThumbnailSize, Qt::KeepAspectRatio), Qt::DecorationRole);
		item->setData(fileName, Qt::DisplayRole);
		appendRow(item);
	}
	//select initial brush
	setCurrentIndex(selectedIndex);
}

int Palapeli::TextureHelper::currentIndex() const
{
	return m_currentIndex;
}

void Palapeli::TextureHelper::setCurrentIndex(int index)
{
	if (m_currentIndex == index)
		return;
	if (index < 0 || index >= rowCount())
		return;
	m_currentIndex = index;
	QBrush brush;
	if (index == 0)
	{
		const QColor color = item(index)->data(BrushRole).value<QColor>();
		m_scene->setBackgroundBrush(item(index)->data(BrushRole).value<QColor>());
		//write config
		Settings::setViewBackgroundColor(color);
		Settings::setViewBackgroundStyle("color");
	}
	else
	{
		m_scene->setBackgroundBrush(item(index)->data(BrushRole).value<QPixmap>());
		//write config
		const QString key = item(index)->data(Qt::DisplayRole).toString();
		Settings::setViewBackground(key);
		Settings::setViewBackgroundStyle("texture");
	}
	Settings::self()->writeConfig();
}

void Palapeli::TextureHelper::setSolidColor(const QColor& color)
{
	QStandardItem* item = this->item(0);
	item->setData(color, BrushRole);
	QPixmap colorThumbnail(DefaultThumbnailSize);
	colorThumbnail.fill(color);
	item->setData(colorThumbnail, Qt::DecorationRole);
	//apply new color and write config if necessary
	if (m_currentIndex == 0)
	{
		m_currentIndex = -1;
		setCurrentIndex(0);
	}
}

#include "texturehelper.moc"

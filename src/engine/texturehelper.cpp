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
#include <QSvgRenderer>

const QSize Palapeli::TextureHelper::DefaultThumbnailSize(32, 32);
const QSize Palapeli::TextureHelper::DefaultPixmapSize(128, 128);

Palapeli::TextureHelper* Palapeli::TextureHelper::instance()
{
	static Palapeli::TextureHelper instance;
	return &instance;
}

QPixmap Palapeli::TextureHelper::render(const QString& fileName)
{
	const QString path = KStandardDirs::locate("appdata", "backgrounds/" + fileName);
	QPixmap pixmap;
	if (fileName.contains(".svg"))
	{
		QSvgRenderer renderer(path);
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

Palapeli::TextureHelper::TextureHelper()
	: m_currentIndex(-1)
{
	//create menu item for solid color
	QPixmap colorThumbnail(DefaultThumbnailSize);
	colorThumbnail.fill(Qt::transparent);
	QStandardItem* colorItem = new QStandardItem;
	colorItem->setData(QLatin1String("__color__"), IdentifierRole);
	colorItem->setData(colorThumbnail, Qt::DecorationRole);
	colorItem->setData(i18nc("@item:inlistbox", "Single color"), Qt::DisplayRole);
	appendRow(colorItem);
	//fetch backgrounds, and create menu items
	const QStringList backgroundFiles = KGlobal::dirs()->findAllResources("appdata", "backgrounds/*");
	foreach (const QString& path, backgroundFiles)
	{
		//get file name and find selected or default backgrounds
		const QString fileName = QFileInfo(path).fileName();
		//create item for this brush
		const QPixmap pixmap = render(fileName);
		QStandardItem* item = new QStandardItem;
		item->setData(pixmap, BrushRole);
		item->setData(fileName, IdentifierRole);
		item->setData(pixmap.scaled(DefaultThumbnailSize, Qt::KeepAspectRatio), Qt::DecorationRole);
		item->setData(fileName, Qt::DisplayRole);
		appendRow(item);
	}
	//select initial brush
	readSettings();
}

int Palapeli::TextureHelper::currentIndex() const
{
	return m_currentIndex;
}

void Palapeli::TextureHelper::readSettings()
{
	//read config
	const QString selectedBackground = Settings::viewBackground();
	const QColor selectedColor = Settings::viewBackgroundColor();
	for (int i = 0; i < rowCount(); ++i)
	{
		QStandardItem* item = this->item(i);
		if (item->data(IdentifierRole) != selectedBackground)
			continue;
		//use this brush
		m_currentIndex = i;
		if (selectedBackground == QLatin1String("__color__"))
			m_currentBrush = selectedColor;
		else
			m_currentBrush = item->data(BrushRole).value<QPixmap>();
		foreach (QGraphicsScene* scene, m_scenes)
			scene->setBackgroundBrush(m_currentBrush);
	}
}

void Palapeli::TextureHelper::addScene(QGraphicsScene* scene)
{
	if (!scene || m_scenes.contains(scene))
		return;
	m_scenes << scene;
	scene->setBackgroundBrush(m_currentBrush);
	connect(scene, SIGNAL(destroyed(QObject*)), SLOT(removeScene(QObject*)));
}

void Palapeli::TextureHelper::removeScene(QObject* scene)
{
	//called by scene->QObject::destroyed signal, so qobject_cast won't work anymore
	m_scenes.removeAll(static_cast<QGraphicsScene*>(scene));
}

#include "texturehelper.moc"

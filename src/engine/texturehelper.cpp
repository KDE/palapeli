/*
    SPDX-FileCopyrightText: 2009, 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "texturehelper.h"
#include "settings.h"

#include <QDir>
#include <QDirIterator>
#include <QGraphicsScene>
#include <QPainter>
#include <KLocalizedString>
#include <QSvgRenderer>

const QSize Palapeli::TextureHelper::DefaultThumbnailSize(32, 32);
const QSize Palapeli::TextureHelper::DefaultPixmapSize(128, 128);

Palapeli::TextureHelper* Palapeli::TextureHelper::instance()
{
	static Palapeli::TextureHelper instance;
	return &instance;
}

QPixmap Palapeli::TextureHelper::render(const QString& filePath)
{
	QPixmap pixmap;
	if (filePath.endsWith(QLatin1String(".svg")))
	{
		QSvgRenderer renderer(filePath);
		pixmap = QPixmap(DefaultPixmapSize);
		pixmap.fill(Qt::transparent);
		QPainter painter(&pixmap);
		renderer.render(&painter);
		painter.end();
	}
	else
		pixmap.load(filePath);
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
	const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::AppLocalDataLocation,
													   QStringLiteral("backgrounds"),
													   QStandardPaths::LocateDirectory);
	for (const QString& dir : dirs)
	{
		QDirIterator dirIt(dir, QDir::Files);
		while (dirIt.hasNext())
		{
			const QString filePath = dirIt.next();
			const QString fileName = dirIt.fileName();
			//create item for this brush
			const QPixmap pixmap = render(filePath);
			if (pixmap.isNull())
				continue;
			QStandardItem* item = new QStandardItem;
			item->setData(pixmap, BrushRole);
			item->setData(fileName, IdentifierRole);
			item->setData(pixmap.scaled(DefaultThumbnailSize, Qt::KeepAspectRatio), Qt::DecorationRole);
			item->setData(fileName, Qt::DisplayRole);
			appendRow(item);
		}
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
		for (QObject* scene : qAsConst(m_scenes))
			static_cast<QGraphicsScene*>(scene)->setBackgroundBrush(m_currentBrush);
	}
}

void Palapeli::TextureHelper::addScene(QGraphicsScene* scene)
{
	if (!scene || m_scenes.contains(scene))
		return;
	m_scenes << scene;
	scene->setBackgroundBrush(m_currentBrush);
	connect(scene, &QObject::destroyed, this, &TextureHelper::removeScene);
}

void Palapeli::TextureHelper::removeScene(QObject* scene)
{
	m_scenes.removeAll(scene);
}



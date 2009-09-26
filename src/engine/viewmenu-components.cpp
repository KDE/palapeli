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

#include "viewmenu-components.h"

#include <QGridLayout>
#include <QPainter>
#include <KStandardDirs>
#include <KSvgRenderer>

const int Palapeli::ViewMenuItem::DefaultButtonSize = 64;
const int Palapeli::ViewMenuItem::DefaultPixmapSize = 128;

//BEGIN Palapeli::ViewMenuItem

Palapeli::ViewMenuItem::ViewMenuItem(const QString& fileName)
	: m_fileName(fileName)
{
	connect(this, SIGNAL(clicked()), this, SLOT(handleClicked()));
	//load/generate pixmap
	const QString path = KStandardDirs::locate("appdata", "backgrounds/" + fileName);
	QPixmap pixmap;
	if (fileName.contains(".svg"))
	{
		KSvgRenderer renderer(path);
		pixmap = QPixmap(QSize(DefaultPixmapSize, DefaultPixmapSize));
		pixmap.fill(Qt::transparent);
		QPainter painter(&pixmap);
		renderer.render(&painter);
		painter.end();
	}
	else
		pixmap.load(path);
	m_brush = pixmap;
	//show pixmap on button
	setIconSize(QSize(DefaultButtonSize, DefaultButtonSize));
	setIcon(pixmap.scaled(DefaultButtonSize, DefaultButtonSize));
}

QBrush Palapeli::ViewMenuItem::brush() const
{
	return m_brush;
}

void Palapeli::ViewMenuItem::enterEvent(QEvent*)
{
	emit startPreview(m_brush);
}

void Palapeli::ViewMenuItem::handleClicked()
{
	emit selected(m_fileName, m_brush);
}

//END Palapeli::ViewMenuItem

//BEGIN Palapeli::ViewMenuWidget

Palapeli::ViewMenuWidget::ViewMenuWidget(const QList<Palapeli::ViewMenuItem*>& items)
{
	QGridLayout* layout = new QGridLayout;
	const int columnCount = 4;
	for (int i = 0; i < items.count(); ++i)
		layout->addWidget(items[i], i / columnCount, i % columnCount);
	setLayout(layout);
}

void Palapeli::ViewMenuWidget::leaveEvent(QEvent* event)
{
	emit stopPreview();
}

//END Palapeli::ViewMenuWidget

#include "viewmenu-components.moc"

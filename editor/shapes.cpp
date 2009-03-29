/***************************************************************************
 *   Copyright 2008 Stefan Majewsky <majewsky@gmx.net>
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

#include "shapes.h"
#include "storage/gamestorage.h"
#include "storage/gamestorageitem.h"

#include <KMessageBox>
#include <KSvgRenderer>
#include <KUrl>

Paladesign::Shapes::Shapes()
	: m_shape(new KSvgRenderer)
{
}

Paladesign::Shapes::~Shapes()
{
	delete m_shape;
}

qreal Paladesign::Shapes::heightForWidth(qreal width) const
{
	static const QString elementId = QLatin1String("paladesign-shape");
	const QRectF bounds = m_shape->boundsOnElement(elementId);
	return bounds.height() / bounds.width() * width;
}

KSvgRenderer* Paladesign::Shapes::shape() const
{
	return m_shape;
}

QUuid Paladesign::Shapes::shapeId() const
{
	return m_shapeId;
}

void Paladesign::Shapes::setShape(const QUuid& id)
{
	static const QString elementId = QLatin1String("paladesign-shape");
	//import item into storage
	Palapeli::GameStorage gs;
	Palapeli::GameStorageItem shapeItem = gs.item(id);
	if (shapeItem.isNull())
		return;
	//load shape
	m_shapeId = id;
	m_shape->load(shapeItem.filePath());
	if (!m_shape->elementExists(elementId))
		KMessageBox::error(0, i18n("Given shape does not contain an element with ID \"paladesign-shape\"."));
	emit shapeChanged();
}

void Paladesign::Shapes::setShape(const KUrl& url)
{
	static const QString elementId = QLatin1String("paladesign-shape");
	//import item into storage
	Palapeli::GameStorage gs;
	Palapeli::GameStorageItem shapeItem = gs.addItem(url, RegularShape);
	if (shapeItem.isNull())
		return;
	//load shape
	m_shapeId = shapeItem.id();
	m_shape->load(shapeItem.filePath());
	if (!m_shape->elementExists(elementId))
		KMessageBox::error(0, ("Given shape does not contain an element with ID \"paladesign-shape\"."));
	emit shapeChanged();
}

#include "shapes.moc"

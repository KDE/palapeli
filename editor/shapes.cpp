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

#include "shapes.h"

#include <KDebug>
#include <KSvgRenderer>

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

void Paladesign::Shapes::setShape(const QString& fileName)
{
	static const QString elementId = QLatin1String("paladesign-shape");
	m_shape->load(fileName);
	if (!m_shape->elementExists(elementId))
		kDebug() << "Given shape does not contain an element with ID \"paladesign-shape\".";
}

void Paladesign::Shapes::setShape(KSvgRenderer* shape)
{
	static const QString elementId = QLatin1String("paladesign-shape");
	if (m_shape != 0)
	{
		delete m_shape;
		m_shape = shape;
		if (!m_shape->elementExists(elementId))
			kDebug() << "Given shape does not contain an element with ID \"paladesign-shape\".";
	}
}

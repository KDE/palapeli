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

#ifndef PALAPELI_ENGINE_BASICS_H
#define PALAPELI_ENGINE_BASICS_H

#include <QGraphicsObject>

namespace Palapeli
{
	//A simple QGraphicsObject subclass with the following features:
	//* automatically enabled qgraphicsitem_cast (template parameters are defined by the GraphicsObjectUserTypes enum)
	//* empty QGraphicsItem reimplementation (i.e. item does not paint anything by default)
	template<int userType> class GraphicsObject : public QGraphicsObject
	{
		public:
			GraphicsObject(bool noPainting = true)
			{ if (noPainting) setFlag(QGraphicsItem::ItemHasNoContents); }

			//empty QGraphicsItem reimplementation
			virtual QRectF boundingRect() const { return QRectF(); }
			virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) {}
			//enable qgraphicsitem_cast
			enum { Type = QGraphicsItem::UserType + userType };
			virtual int type() const { return Type; }
	};

	enum GraphicsObjectUserTypes
	{
		GeneralUserType = 0,
		PieceUserType = 1,
		PartUserType = 2,
		ShadowUserType = 3,
		ConstraintVisualizerUserType = 11
	};

	typedef Palapeli::GraphicsObject<Palapeli::GeneralUserType> EmptyGraphicsObject;
}

#endif // PALAPELI_ENGINE_BASICS_H

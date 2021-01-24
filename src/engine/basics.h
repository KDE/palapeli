/*
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
			QRectF boundingRect() const override { return QRectF(); }
			void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override {}
			//enable qgraphicsitem_cast
			enum { Type = QGraphicsItem::UserType + userType };
			int type() const override { return Type; }
	};

	enum GraphicsObjectUserTypes
	{
		GeneralUserType = 0,
		PieceUserType = 1,
		ConstraintVisualizerUserType = 11
	};

	typedef Palapeli::GraphicsObject<Palapeli::GeneralUserType> EmptyGraphicsObject;
}

#endif // PALAPELI_ENGINE_BASICS_H

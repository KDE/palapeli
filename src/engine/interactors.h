/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef PALAPELI_INTERACTORS_H
#define PALAPELI_INTERACTORS_H

#include "interactor.h"

#include <QGraphicsItem>

namespace Palapeli
{
	class MoveViewportInteractor : public Palapeli::Interactor
	{
		public:
			MoveViewportInteractor(QGraphicsView* view);
		protected:
			virtual void mouseMoveEvent(const Palapeli::InteractorMouseEvent& event);
	};

	class ZoomViewportInteractor : public Palapeli::Interactor
	{
		public:
			ZoomViewportInteractor(QGraphicsView* view);
		protected:
			virtual void wheelEvent(const Palapeli::InteractorWheelEvent& event);
	};

	class RubberBandItem : public QGraphicsItem
	{
		public:
			RubberBandItem(QGraphicsItem* parent = 0);

			QRectF rect() const;
			void setRect(const QRectF& rect);

			virtual QRectF boundingRect() const;
			virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);
		private:
			QRectF m_rect;
	};

	class RubberBandInteractor : public Palapeli::Interactor
	{
		public:
			RubberBandInteractor(QGraphicsView* view);
			virtual ~RubberBandInteractor();
		protected:
			virtual void sceneChangeEvent(QGraphicsScene* oldScene, QGraphicsScene* newScene);
			virtual bool acceptItemUnderMouse(QGraphicsItem* item);
			virtual void mousePressEvent(const Palapeli::InteractorMouseEvent& event);
			virtual void mouseMoveEvent(const Palapeli::InteractorMouseEvent& event);
			virtual void mouseReleaseEvent(const Palapeli::InteractorMouseEvent& event);
		private:
			Palapeli::RubberBandItem* m_item;
			QPointF m_basePosition;
	};
}

#endif // PALAPELI_INTERACTORS_H

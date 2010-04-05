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

#ifndef PALAPELI_SHADOWITEM_H
#define PALAPELI_SHADOWITEM_H

#include "basics.h"

class QPropertyAnimation;

namespace Palapeli
{
	QPixmap makePixmapMonochrome(const QPixmap& pixmap, const QColor& color);
	QPixmap createShadow(const QPixmap& source, int radius);

	class ShadowItem : public Palapeli::GraphicsObject<Palapeli::ShadowUserType>
	{
		Q_OBJECT
		Q_PROPERTY(qreal activeOpacity READ activeOpacity WRITE setActiveOpacity)
		public:
			ShadowItem(const QPixmap& pixmap, int radius, const QPointF& offset);

			qreal activeOpacity() const;
			void setActiveOpacity(qreal activeOpacity);
		public Q_SLOTS:
			void setActive(bool active);
		private:
			QGraphicsPixmapItem* m_baseShadow;
			QGraphicsPixmapItem* m_activeShadow;
			QPropertyAnimation* m_animator;
	};
}

#endif // PALAPELI_SHADOWITEM_H

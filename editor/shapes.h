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

#ifndef PALADESIGN_SHAPES_H
#define PALADESIGN_SHAPES_H

#include "../storage/gamestorageitem.h"

#include <QObject>
#include <QRectF>
class QString;
#include <QUuid>
class KSvgRenderer;
class KUrl;

namespace Paladesign
{

	class Shapes : public QObject
	{
		Q_OBJECT
		public:
			Shapes();
			~Shapes();

			qreal heightForWidth(qreal width) const;
			KSvgRenderer* shape() const;
			QUuid shapeId() const;

			void setShape(const QUuid& id);
			void setShape(const KUrl& url);

			enum GameStorageType //extension to the Palapeli::GameStorageItemType enumeration
			{
				RegularShape = Palapeli::GameStorageItem::UserType + 1
			};
		Q_SIGNALS:
			void shapeChanged();
		private:
			KSvgRenderer* m_shape;
			QUuid m_shapeId;
	};

}

#endif // PALADESIGN_SHAPES_H

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

#ifndef PALAPELI_VIEW_H
#define PALAPELI_VIEW_H

#include <QGraphicsView>

namespace Palapeli
{
	class InaccessibleAreasHelper;
	class Scene;
	class TextureHelper;

	class View : public QGraphicsView
	{
		Q_OBJECT
		public:
			View();
			virtual ~View();

			Palapeli::Scene* scene() const;
			Palapeli::TextureHelper* textureHelper() const;
		public Q_SLOTS:
			void zoomIn();
			void zoomOut();
			void zoomBy(int delta); //delta = 0 -> no change, delta < 0 -> zoom out, delta > 0 -> zoom in
			void zoomTo(qreal level); //level = 1 -> show complete scene rect, allowed values: 1 <= level <= 10
		protected:
			virtual void resizeEvent(QResizeEvent* event);
			virtual void wheelEvent(QWheelEvent* event);
			void restrictViewportToSceneRect();
		Q_SIGNALS:
			void zoomLevelChanged(qreal level);
		private Q_SLOTS:
			void sceneRectChanged(const QRectF& sceneRect);
		private:
			Palapeli::Scene* m_scene;
			Palapeli::InaccessibleAreasHelper* m_iaHelper;
			Palapeli::TextureHelper* m_txHelper;
			qreal m_zoomLevel;
	};
}

#endif // PALAPELI_VIEW_H

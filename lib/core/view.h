/***************************************************************************
 *   Copyright (C) 2008 Felix Lemke <lemke.felix@ages-skripte.org>
 *   Copyright (C) 2008-2009 Stefan Majewsky <majewsky@gmx.net>
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

#include "../macros.h"

class QBrush;
#include <QGraphicsView>

namespace Palapeli
{

	class ViewMenu;

	class PALAPELIBASE_EXPORT View : public QGraphicsView
	{
		Q_OBJECT
		public:
			View(QWidget* parent = 0);
			virtual ~View();

			void setAntialiasing(bool antialiasing, bool forceApplication = false);
			void setHardwareAccelerated(bool useHardware, bool forceApplication = false);

			Palapeli::ViewMenu* menu() const;
			QGraphicsScene* realScene() const;
			void useScene(bool useScene);
			void moveToTop(QGraphicsItem* item) const;
			void scale(qreal scalingFactor);
		Q_SIGNALS:
			void viewportMoved();
			void viewportScaled();
		protected:
			virtual void contextMenuEvent(QContextMenuEvent* event);
			virtual void resizeEvent(QResizeEvent* event);
			virtual void wheelEvent(QWheelEvent* event);
		private Q_SLOTS:
			void updateBackground(const QBrush& brush);
			void startBackgroundPreview(const QBrush& brush);
			void endBackgroundPreview();
		private:
			Palapeli::ViewMenu* m_menu;
			QGraphicsScene* m_scene;
			QBrush m_backgroundBrush; //saves the current brush while previewing another one
	};

}

#endif //PALAPELI_VIEW_H

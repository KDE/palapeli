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

#ifndef PALAPELI_TEXTUREHELPER_H
#define PALAPELI_TEXTUREHELPER_H

class QGraphicsScene;
#include <QStandardItemModel>

namespace Palapeli
{
	class TextureHelper : public QStandardItemModel
	{
		Q_OBJECT
		public:
			enum CustomRoles {
				BrushRole = Qt::UserRole + 1,
				StyleRole = Qt::UserRole + 2
			};

			TextureHelper(QGraphicsScene* scene);

			int currentIndex() const;
		public Q_SLOTS:
			void setCurrentIndex(int index);
			void setSolidColor(const QColor& color);
		private:
			static QPixmap render(const QString& fileName);

			QGraphicsScene* m_scene;
			int m_currentIndex;

			static const QSize DefaultThumbnailSize;
			static const QSize DefaultPixmapSize;
	};
}

#endif // PALAPELI_TEXTUREHELPER_H

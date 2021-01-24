/*
    SPDX-FileCopyrightText: 2009, 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELI_TEXTUREHELPER_H
#define PALAPELI_TEXTUREHELPER_H

class QGraphicsScene;
#include <QStandardItemModel>

namespace Palapeli
{
	//NOTE: Singleton, needs to be initialized in main().
	class TextureHelper : public QStandardItemModel
	{
		Q_OBJECT
		public:
			enum CustomRoles {
				BrushRole = Qt::UserRole + 1,
				IdentifierRole = Qt::UserRole + 2
			};

			static Palapeli::TextureHelper* instance();
			int currentIndex() const;
		public Q_SLOTS:
			void readSettings();
			void addScene(QGraphicsScene* scene);
			void removeScene(QObject* object);
		private:
			TextureHelper();
			static QPixmap render(const QString& filePath);

			QList<QObject*> m_scenes;
			int m_currentIndex;
			QBrush m_currentBrush;

			static const QSize DefaultThumbnailSize;
			static const QSize DefaultPixmapSize;
	};
}

#endif // PALAPELI_TEXTUREHELPER_H

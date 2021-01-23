/***************************************************************************
 *   Copyright 2010 Johannes Loehnert <loehnert.kde@gmx.de>
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

#ifndef PALAPELI_PUZZLEPREVIEW_H
#define PALAPELI_PUZZLEPREVIEW_H

#include <QGraphicsView>
#include <QTimer>

namespace Palapeli
{
	struct PuzzleMetadata;

	class PuzzlePreview : public QGraphicsView
	{
		Q_OBJECT
		public:
			explicit PuzzlePreview(QWidget* parent);

			void setImage(const QImage &image);
			void loadImageFrom(const Palapeli::PuzzleMetadata& md);

		public Q_SLOTS:
			// toggles visibility state AND updates config with the new state.
			void toggleVisible();

		Q_SIGNALS:
			void closing();

		protected:
			void mouseMoveEvent(QMouseEvent* event) override;
			void enterEvent(QEvent* event) override;
			void leaveEvent(QEvent* event) override;
			void resizeEvent(QResizeEvent* event) override;
			void moveEvent(QMoveEvent *event) override;
			void closeEvent(QCloseEvent* event) override;
			void updateViewport();

		private Q_SLOTS:
			void writeConfigIfGeometryChanged();

		private:
			// used to save geometry after move/resize, to avoid writing config file each time the cursor moves a pixel.
			QTimer* m_settingsSaveTimer;
			bool m_geometryChanged;

			qreal m_hoverZoom;
			bool m_isZoomed;
			QPoint m_mousePos;
	};
}

#endif // PALAPELI_PUZZLEPREVIEW_H

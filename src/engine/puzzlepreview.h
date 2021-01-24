/*
    SPDX-FileCopyrightText: 2010 Johannes Loehnert <loehnert.kde@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

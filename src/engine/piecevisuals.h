/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>
    SPDX-FileCopyrightText: 2010 Johannes Löhnert <loehnert.kde@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELI_PIECEVISUALS_H
#define PALAPELI_PIECEVISUALS_H

#include <QPixmap>

namespace Palapeli
{
	struct BevelPoint
	{
		BevelPoint() : strength(0), angle(0), orig_argb(0) {}
		// gives the strength of the bevel at this point, i.e. the amount of darken/lighten
		char strength;
		// full circle is divided into 256 units. The translate into usual degrees like this:
		// 256 => 360°, 64 => 90° ...
		// 0 is at 3 o'clock and +x is ccw.
		char angle;
		// color of unaltered pixmap at this point.
		// if strength=0, orig_color stores not the color, but the number of
		// consecutive 0 points (including the current),
		// providing a simple RLE for those runs to speed up things later.
		QRgb orig_argb;
	};

	// Note that width and height are not stored. For this one needs the original pixmap
	// where the BevelMap belongs to.
	typedef QVector<BevelPoint> BevelMap;

	class PieceVisuals
	{
		public:
			PieceVisuals() {}
			PieceVisuals(const QImage& image, const QPoint& offset) : m_image(image), m_offset(offset) {}
			PieceVisuals(const QPixmap& pixmap, const QPoint& offset) : m_pixmap(pixmap), m_offset(offset) {}

			inline bool hasImage() const { return !m_image.isNull(); }
			inline bool hasPixmap() const { return !m_pixmap.isNull(); }
			inline bool isNull() const { return m_image.isNull() && m_pixmap.isNull(); }
			inline QPoint offset() const { return m_offset; }

			inline QImage image() const
			{
				if (m_image.isNull())
					m_image = m_pixmap.toImage();
				return m_image;
			}
			inline QPixmap pixmap() const
			{
				if (m_pixmap.isNull())
					m_pixmap = QPixmap::fromImage(m_image);
				return m_pixmap;
			}
			inline QSize size() const
			{
				//One of them might be (0,0) if no conversion has happened yet.
				return m_pixmap.size().expandedTo(m_image.size());
			}
		private:
			mutable QImage m_image;
			mutable QPixmap m_pixmap;
			QPoint m_offset;

	};

	//TODO: Many of these functions operate on QImages internally, and could therefore be more efficient when given a QImage.
	Palapeli::BevelMap calculateBevelMap(const QImage& source, int radius);
	QImage applyBevelMap(const QImage& source, const Palapeli::BevelMap& bevelmap, qreal angle);
	Palapeli::PieceVisuals createShadow(const Palapeli::PieceVisuals& pieceVisuals, const QSize& shadowSizeHint = QSize());
	Palapeli::PieceVisuals changeShadowColor(const Palapeli::PieceVisuals& shadowVisuals, const QColor& color);
	Palapeli::PieceVisuals mergeVisuals(const QList<Palapeli::PieceVisuals>& visuals);
	// visuals are needed to determine positioning of maps
	Palapeli::BevelMap mergeBevelMaps(const QList<Palapeli::PieceVisuals>& visuals, const QList<Palapeli::BevelMap>& bevelMaps);
}

#endif // PALAPELI_PIECEVISUALS_H

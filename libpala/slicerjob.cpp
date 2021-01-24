/*
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "slicerjob.h"
#include "slicer.h"

#include <QPainter>

//BEGIN Pala::SlicerJobPrivate

class Pala::SlicerJobPrivate
{
	public:
		SlicerJobPrivate() : m_mode(nullptr) {}

		QMap<QByteArray, QVariant> m_args;
		QImage m_image;
		const Pala::SlicerMode* m_mode;

		QMap<int, QImage> m_pieces;
		QMap<int, QPoint> m_pieceOffsets;
		QList<QPair<int, int> > m_relations;
};

//END Pala::SlicerJobPrivate

Pala::SlicerJob::SlicerJob(const QImage& image, const QMap<QByteArray, QVariant>& args)
	: d_ptr(new SlicerJobPrivate)
{
	Q_D(SlicerJob);
	d->m_args = args;
	d->m_image = image;
}

Pala::SlicerJob::~SlicerJob() = default;

QVariant Pala::SlicerJob::argument(const QByteArray& key) const
{
	Q_D(const SlicerJob);
	return d->m_args.value(key, QVariant());
}

QImage Pala::SlicerJob::image() const
{
	Q_D(const SlicerJob);
	return d->m_image;
}

const Pala::SlicerMode* Pala::SlicerJob::mode() const
{
	Q_D(const SlicerJob);
	return d->m_mode;
}

QMap<int, QImage> Pala::SlicerJob::pieces() const
{
	Q_D(const SlicerJob);
	return d->m_pieces;
}

QMap<int, QPoint> Pala::SlicerJob::pieceOffsets() const
{
	Q_D(const SlicerJob);
	return d->m_pieceOffsets;
}

QList<QPair<int, int> > Pala::SlicerJob::relations() const
{
	Q_D(const SlicerJob);
	return d->m_relations;
}

void Pala::SlicerJob::setMode(const Pala::SlicerMode* mode)
{
	Q_D(SlicerJob);
	d->m_mode = mode;
}

void Pala::SlicerJob::respectSlicerFlags(int flags)
{
	Q_D(SlicerJob);
	if (!(flags & Pala::Slicer::AllowFullTransparency))
	{
		QImage image(d->m_image.size(), d->m_image.format());
		QColor blackShade(Qt::black); blackShade.setAlpha(42);
		image.fill(blackShade.rgba());
		QPainter painter(&image);
		painter.drawImage(QPoint(), d->m_image);
		painter.end();
		d->m_image = image;
	}
}

void Pala::SlicerJob::addPiece(int pieceID, const QImage& image, const QPoint& offset)
{
	Q_D(SlicerJob);
	d->m_pieces.insert(pieceID, image);
	d->m_pieceOffsets.insert(pieceID, offset);
}

//A modified version of QImage::copy, which avoids rendering errors even if rect is outside the bounds of the source image.
static
QImage safeQImageCopy(const QImage& source, const QRect& rect)
{
	QRect targetRect(QPoint(), rect.size());
	//copy image
	QImage target(rect.size(), source.format());
	QPainter p(&target);
	p.drawImage(targetRect, source, rect);
	p.end();
	return target;
	//Strangely, source.copy(rect) does not work. It produces black borders.
}

void Pala::SlicerJob::addPieceFromMask(int pieceID, const QImage& mask, const QPoint& offset)
{
	Q_D(SlicerJob);
	QImage pieceImage(mask);
	QPainter painter(&pieceImage);
	painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
	painter.drawImage(QPoint(), safeQImageCopy(d->m_image, QRect(offset, mask.size())));
	painter.end();
	addPiece(pieceID, pieceImage, offset);
}

void Pala::SlicerJob::addRelation(int pieceID1, int pieceID2)
{
	Q_D(SlicerJob);
	d->m_relations << QPair<int, int>(pieceID1, pieceID2);
}

/*
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "slicerjob.h"
#include "slicer.h"

#include <QPainter>

//BEGIN Pala::SlicerJob::Private

class Pala::SlicerJob::Private
{
	public:
		Private() : m_mode(nullptr) {}

		QMap<QByteArray, QVariant> m_args;
		QImage m_image;
		const Pala::SlicerMode* m_mode;

		QMap<int, QImage> m_pieces;
		QMap<int, QPoint> m_pieceOffsets;
		QList<QPair<int, int> > m_relations;
};

//END Pala::SlicerJob::Private

Pala::SlicerJob::SlicerJob(const QImage& image, const QMap<QByteArray, QVariant>& args)
	: p(new Private)
{
	p->m_args = args;
	p->m_image = image;
}

Pala::SlicerJob::~SlicerJob()
{
	delete p;
}

QVariant Pala::SlicerJob::argument(const QByteArray& key) const
{
	return p->m_args.value(key, QVariant());
}

QImage Pala::SlicerJob::image() const
{
	return p->m_image;
}

const Pala::SlicerMode* Pala::SlicerJob::mode() const
{
	return p->m_mode;
}

QMap<int, QImage> Pala::SlicerJob::pieces() const
{
	return p->m_pieces;
}

QMap<int, QPoint> Pala::SlicerJob::pieceOffsets() const
{
	return p->m_pieceOffsets;
}

QList<QPair<int, int> > Pala::SlicerJob::relations() const
{
	return p->m_relations;
}

void Pala::SlicerJob::setMode(const Pala::SlicerMode* mode)
{
	p->m_mode = mode;
}

void Pala::SlicerJob::respectSlicerFlags(int flags)
{
	if (!(flags & Pala::Slicer::AllowFullTransparency))
	{
		QImage image(p->m_image.size(), p->m_image.format());
		QColor blackShade(Qt::black); blackShade.setAlpha(42);
		image.fill(blackShade.rgba());
		QPainter painter(&image);
		painter.drawImage(QPoint(), p->m_image);
		painter.end();
		p->m_image = image;
	}
}

void Pala::SlicerJob::addPiece(int pieceID, const QImage& image, const QPoint& offset)
{
	p->m_pieces.insert(pieceID, image);
	p->m_pieceOffsets.insert(pieceID, offset);
}

//A modified version of QImage::copy, which avoids rendering errors even if rect is outside the bounds of the source image.
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
	QImage pieceImage(mask);
	QPainter painter(&pieceImage);
	painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
	painter.drawImage(QPoint(), safeQImageCopy(p->m_image, QRect(offset, mask.size())));
	painter.end();
	addPiece(pieceID, pieceImage, offset);
}

void Pala::SlicerJob::addRelation(int pieceID1, int pieceID2)
{
	p->m_relations << QPair<int, int>(pieceID1, pieceID2);
}

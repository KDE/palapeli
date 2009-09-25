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

#include "slicerjob.h"

#include <QtGui/QPainter>

//BEGIN Pala::SlicerJob::Private

class Pala::SlicerJob::Private
{
	public:
		QMap<QByteArray, QVariant> m_args;
		QImage m_image;

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

void Pala::SlicerJob::addPiece(int pieceID, const QImage& image, const QPoint& offset)
{
	p->m_pieces.insert(pieceID, image);
	p->m_pieceOffsets.insert(pieceID, offset);
}

void Pala::SlicerJob::addPieceFromMask(int pieceID, const QImage& mask, const QPoint& offset)
{
	QImage pieceImage(mask);
	QPainter painter(&pieceImage);
	painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
	painter.drawImage(-offset, p->m_image);
	painter.end();
	addPiece(pieceID, pieceImage, offset);
}

void Pala::SlicerJob::addRelation(int pieceID1, int pieceID2)
{
	p->m_relations << QPair<int, int>(pieceID1, pieceID2);
}

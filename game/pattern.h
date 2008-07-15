/***************************************************************************
 *   Copyright (C) 2008 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef PALAPELI_PATTERN_H
#define PALAPELI_PATTERN_H

class QImage;
#include <QObject>
class QPointF;
class QPixmap;
class QRectF;

namespace Palapeli
{

	class PatternPrivate;

	class Pattern : public QObject
	{
		//TODO: documentation (I will include that once this class move into installable headers)
		//TODO: signals for progress reporting
		Q_OBJECT
		public:
			Pattern();
			virtual ~Pattern();

			//implementation of subclasses (i.e. plugins); interface to Palapeli core
			virtual void slice(const QImage& image) = 0;
		protected:
			//interface to subclasses (i.e. plugins)
			void addPiece(const QPixmap& pixmap, const QRectF& positionInImage);
			void addRelation(int piece1Id, int piece2Id, const QPointF& positionDifference);
		private:
			PatternPrivate* p;
	};
}

#endif // PALAPELI_PATTERN_H

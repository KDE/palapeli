/***************************************************************************
 *   Copyright 2010 Stefan Majewsky <majewsky@gmx.net>
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

#ifndef PALAPELI_MERGEGROUP_H
#define PALAPELI_MERGEGROUP_H

class QGraphicsScene;
#include <QObject>
#include <QPointF>

namespace Palapeli
{
	class Piece;

	///This transaction class handles merging of physically neighboring pieces.
	class MergeGroup : public QObject
	{
		Q_OBJECT
		public:
			static QList<Palapeli::Piece*> tryGrowMergeGroup(Palapeli::Piece* piece);

			///If \a animated is set, the transaction will wait for the animation to finish and then fire the pieceInstanceTransaction() signal. After this emission, the MergeGroup will delete itself.
			///If \a animated is not set, you have to obtain the generated piece manually from the mergedPiece() method.
			MergeGroup(const QList<Palapeli::Piece*>& pieces, QGraphicsScene* scene, bool animated = true);

			Palapeli::Piece* mergedPiece() const;
		Q_SIGNALS:
			void pieceInstanceTransaction(const QList<Palapeli::Piece*>& deletedPieces, const QList<Palapeli::Piece*>& createdPieces);
		private:
			QList<Palapeli::Piece*> m_pieces;
			Palapeli::Piece* m_mergedPiece;
			//parameters of united coordinate system (UCS)
			QPointF m_ucsPosition;
	};
}

#endif // PALAPELI_MERGEGROUP_H

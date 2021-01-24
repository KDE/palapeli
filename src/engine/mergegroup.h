/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELI_MERGEGROUP_H
#define PALAPELI_MERGEGROUP_H

class QGraphicsScene;
#include <QObject>
#include <QPointF>
#include <QSizeF>

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
			MergeGroup(const QList<Palapeli::Piece*>& pieces, QGraphicsScene* scene, const QSizeF& pieceAreaSize, bool animated = true);

			void start();

			///Will return 0 if the animation is still in progress.
			Palapeli::Piece* mergedPiece() const;
		Q_SIGNALS:
			void pieceInstanceTransaction(const QList<Palapeli::Piece*>& deletedPieces, const QList<Palapeli::Piece*>& createdPieces);
		private Q_SLOTS:
			void createMergedPiece();
		private:
			bool m_animated;
			QList<Palapeli::Piece*> m_pieces;
			Palapeli::Piece* m_mergedPiece;
			QGraphicsScene* m_scene;
			//parameters of united coordinate system (UCS)
			QPointF m_ucsPosition;
			QSizeF m_pieceAreaSize;
	};
}

#endif // PALAPELI_MERGEGROUP_H

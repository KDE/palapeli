/*
    SPDX-FileCopyrightText: 2010 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PALAPELI_CONSTRAINTINTERACTOR_H
#define PALAPELI_CONSTRAINTINTERACTOR_H

#include "interactor.h"

namespace Palapeli
{
	class ConstraintInteractor : public Palapeli::Interactor
	{
		public:
			explicit ConstraintInteractor(QGraphicsView* view);
		protected:
			bool startInteraction(const Palapeli::MouseEvent& event) override;
			void continueInteraction(const Palapeli::MouseEvent& event) override;
			void stopInteraction(const Palapeli::MouseEvent& event) override;
		private:
			enum Side { LeftSide = 0, RightSide, TopSide, BottomSide };
			QList<Side> touchingSides(const QPointF& scenePos) const;

			QList<Side> m_draggingSides;
			QPointF m_baseSceneRectOffset;
	};
}

#endif // PALAPELI_CONSTRAINTINTERACTOR_H

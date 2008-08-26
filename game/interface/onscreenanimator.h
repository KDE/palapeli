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

#ifndef PALAPELI_ONSCREENANIMATOR_H
#define PALAPELI_ONSCREENANIMATOR_H

#include <QObject>
class QTimeLine;

namespace Palapeli
{

	class OnScreenWidget;

	class OnScreenAnimator : public QObject
	{
		Q_OBJECT
		public:
			enum Direction
			{
				NoDirection,
				ShowDirection,
				HideDirection
			};

			OnScreenAnimator(OnScreenWidget* widget);
			~OnScreenAnimator();

			Direction direction() const;
			int duration() const; //in milliseconds
		public Q_SLOTS:
			void start(Direction direction);
		Q_SIGNALS:
			void finished();
		private Q_SLOTS:
			void changeValue1(qreal value);
			void changeValue2(qreal value);
			void changeValuePosition(qreal value);
			void changeValueOpacity(qreal value);
			void animationEnd1();
			void animationEnd2();
		private:
			QTimeLine* m_timeLine;
			OnScreenWidget* m_widget;
			Direction m_direction;
	};

}

#endif // PALAPELI_ONSCREENANIMATOR_H

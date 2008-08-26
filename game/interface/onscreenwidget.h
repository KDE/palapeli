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

#ifndef PALAPELI_ONSCREENWIDGET_H
#define PALAPELI_ONSCREENWIDGET_H

class QGraphicsProxyWidget;
class QGraphicsView;
#include <QGraphicsWidget>
class QWidget;

//FIXME: OnScreenWidget does not correctly adapt to changes in the contained widget's size hint...
//       This seems to be a problem with updateGeometry(), but as I do not see a solution for now,
//       I'll stick with the workaround to not change the widget's size hint after its creation.

namespace Palapeli
{

	class AutoscalingItem;
	class OnScreenAnimator;

	class OnScreenWidget : public QGraphicsWidget
	{
		Q_OBJECT
		public:
			OnScreenWidget(QWidget* widget, Palapeli::AutoscalingItem* parent = 0); //takes ownership of widget
			~OnScreenWidget();

			const OnScreenAnimator* animator() const;

			void setWidget(QWidget* widget);

			virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);
			virtual void setGeometry(const QRectF& rect);
		public Q_SLOTS:
			void showAnimated();
			void hideAnimated();
		protected:
			virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF& constraint = QSizeF()) const;
			virtual void hideEvent(QHideEvent* event);
			virtual void showEvent(QShowEvent* event);
		private:
			Q_DISABLE_COPY(OnScreenWidget)
			QGraphicsProxyWidget* m_proxy;
			OnScreenAnimator* m_animator;
	};

}

#endif // PALAPELI_ONSCREENWIDGET_H

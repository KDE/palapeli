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

	class OnScreenAnimator;

	class OnScreenWidget : public QGraphicsWidget
	{
		Q_OBJECT
		public:
			OnScreenWidget(QWidget* widget, QGraphicsItem* parent = 0); //takes ownership of widget
			~OnScreenWidget();

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

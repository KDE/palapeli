#ifndef PALAPELI_ONSCREENANIMATOR_H
#define PALAPELI_ONSCREENANIMATOR_H

#include <QTimeLine>

namespace Palapeli
{

	class OnScreenWidget;

	class OnScreenAnimator : public QTimeLine
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
		public Q_SLOTS:
			void start(Direction direction);
		private Q_SLOTS:
			void changeValue(qreal value);
		private:
			OnScreenWidget* m_widget;
			Direction m_direction;
	};

}

#endif // PALAPELI_ONSCREENANIMATOR_H

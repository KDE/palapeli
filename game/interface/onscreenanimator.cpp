#include "onscreenanimator.h"
#include "onscreenwidget.h"

#include <QGraphicsView>

Palapeli::OnScreenAnimator::OnScreenAnimator(Palapeli::OnScreenWidget* widget)
	: QTimeLine(500)
	, m_widget(widget)
	, m_direction(NoDirection)
{
	setFrameRange(0, 15); //that is 30 fps which should be sufficient
	connect(this, SIGNAL(valueChanged(qreal)), this, SLOT(changeValue(qreal)));
}

void Palapeli::OnScreenAnimator::changeValue(qreal value)
{
	//TODO: LTR/RTL
	if (!m_widget)
		return;
	//find base point of animation, and size of widget
	QList<QGraphicsView*> views = m_widget->scene()->views();
	if (views.isEmpty())
		return;
	const QPointF basePoint = views[0]->mapToScene(0, 0);
	const QSizeF widgetSize = m_widget->boundingRect().size();
	//find start and end point of animation
	QPointF startPoint, endPoint;
	switch (m_direction)
	{
		case NoDirection:
			startPoint = endPoint = basePoint;
			break;
		case ShowDirection:
			startPoint = QPointF(basePoint.x() - widgetSize.width(), basePoint.y() - widgetSize.height());
			endPoint = basePoint;
			break;
		case HideDirection:
			startPoint = basePoint;
			endPoint = QPointF(basePoint.x() - widgetSize.width(), basePoint.y() - widgetSize.height());
			break;
	}
	m_widget->setPos(startPoint + value * (endPoint - startPoint));
}

void Palapeli::OnScreenAnimator::start(Palapeli::OnScreenAnimator::Direction direction)
{
	m_direction = direction;
	QTimeLine::start();
	changeValue(0); //move widget to start point
	m_widget->show(); //make sure the widget is visible during the animation
}

#include "onscreenanimator.moc"

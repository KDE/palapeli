#include "view.h"
#include "scene.h"

#include <QScrollBar>
#include <QWheelEvent>

Palapeli::View::View(QWidget* parent)
	: QGraphicsView(parent)
	, m_scene(0)
{
	//setDragMode(QGraphicsView::ScrollHandDrag);
}

Palapeli::View::~View()
{
	delete m_scene;
}

void Palapeli::View::startGame(int sceneWidth, int sceneHeight, const QString &fileName, int xPieces, int yPieces)
{
	delete m_scene;
	m_scene = new Palapeli::Scene(sceneWidth, sceneHeight);
	m_scene->loadImage(fileName, xPieces, yPieces);
	setScene(m_scene);
}

void Palapeli::View::wheelEvent(QWheelEvent* event)
{
	qreal delta = event->delta();
	if (event->modifiers() & Qt::ControlModifier)
	{
		//control + mouse wheel - zoom viewport in/out
		const qreal deltaAdaptationFactor = 600.0;
		const qreal scaleBy = qAbs(delta) / deltaAdaptationFactor;
		if (delta < 0)
			scale(1 - scaleBy, 1 - scaleBy);
		else
			scale(1 + scaleBy, 1 + scaleBy);
	}
	else if (event->modifiers() & Qt::ShiftModifier)
	{
		//shift + mouse wheel - move the viewport left/right by adjusting the slider
		horizontalScrollBar()->triggerAction(delta < 0 ? QAbstractSlider::SliderSingleStepAdd : QAbstractSlider::SliderSingleStepSub);
	}
	else
	{
		//just the mouse wheel - move the viewport up/down by adjusting the slider
		verticalScrollBar()->triggerAction(delta < 0 ? QAbstractSlider::SliderSingleStepAdd : QAbstractSlider::SliderSingleStepSub);
	}
}

#include "view.moc"

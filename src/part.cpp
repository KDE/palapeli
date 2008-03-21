#include "part.h"
#include "piece.h"
#include "scene.h"

#include <QGraphicsSceneMouseEvent>

Palapeli::Part::Part(Palapeli::Piece* piece, Scene* scene)
	: QGraphicsItem()
	, m_scene(scene)
	, m_moving(false)
{
	piece->setPart(this);
	piece->setPos(0.0, 0.0);
	setAcceptedMouseButtons(Qt::NoButton);
}

Palapeli::Part::~Part()
{
	//The pieces are destroyed by the scene.
}

QRectF Palapeli::Part::boundingRect() const
{
	QRectF rect(0.0, 0.0, 0.0, 0.0);
	foreach(Palapeli::Piece* piece, m_pieces)
		rect = rect.united(piece->mapToParent(piece->boundingRect()).boundingRect());
	return rect;
}

void Palapeli::Part::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*)
{
}

void Palapeli::Part::mousePressEvent(QGraphicsSceneMouseEvent*)
{
	m_moving = true;
}

void Palapeli::Part::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	if (event->button() & Qt::LeftButton)
		m_moving = true; 
	if (m_moving)
	{
		QPointF pos = event->scenePos();
		QPointF lastPos = event->lastScenePos();
		moveBy(pos.x() - lastPos.x(), pos.y() - lastPos.y());
	}
}

void Palapeli::Part::mouseReleaseEvent(QGraphicsSceneMouseEvent*)
{
	m_moving = false;
	//search for combinations
	foreach (Palapeli::Piece* piece, m_pieces)
	{
		int xIndex = piece->xIndex(), yIndex = piece->yIndex();
		QRectF myRect = piece->sceneBoundingRect();
		qreal xMaxInaccuracy = 0.1 * myRect.width();
		qreal yMaxInaccuracy = 0.1 * myRect.height();
		Palapeli::Piece *right = m_scene->rightNeighbor(xIndex, yIndex);
		if (right != 0 && piece->parentItem() != right->parentItem())
		{
			QRectF otherRect = right->sceneBoundingRect();
			qreal dx = otherRect.x() - myRect.x() - myRect.width();
			qreal dy = otherRect.y() - myRect.y();
			if (qAbs(dx) <= xMaxInaccuracy && qAbs(dy) <= yMaxInaccuracy)
				m_scene->combineParts(this, right->part(), -dx, -dy);
		}
		Palapeli::Piece *left = m_scene->leftNeighbor(xIndex, yIndex);
		if (left != 0 && piece->parentItem() != left->parentItem())
		{
			QRectF otherRect = left->sceneBoundingRect();
			qreal dx = otherRect.x() - myRect.x() + myRect.width();
			qreal dy = otherRect.y() - myRect.y();
			if (qAbs(dx) <= xMaxInaccuracy && qAbs(dy) <= yMaxInaccuracy)
				m_scene->combineParts(this, left->part(), -dx, -dy);
		}
		Palapeli::Piece *top = m_scene->topNeighbor(xIndex, yIndex);
		if (top != 0 && piece->parentItem() != top->parentItem())
		{
			QRectF otherRect = top->sceneBoundingRect();
			qreal dx = otherRect.x() - myRect.x();
			qreal dy = otherRect.y() - myRect.y() + myRect.height();
			if (qAbs(dx) <= xMaxInaccuracy && qAbs(dy) <= yMaxInaccuracy)
				m_scene->combineParts(this, top->part(), -dx, -dy);
		}
		Palapeli::Piece *bottom = m_scene->bottomNeighbor(xIndex, yIndex);
		if (bottom != 0 && piece->parentItem() != bottom->parentItem())
		{
			QRectF otherRect = bottom->sceneBoundingRect();
			qreal dx = otherRect.x() - myRect.x();
			qreal dy = otherRect.y() - myRect.y() - myRect.height();
			if (qAbs(dx) <= xMaxInaccuracy && qAbs(dy) <= yMaxInaccuracy)
				m_scene->combineParts(this, bottom->part(), -dx, -dy);
		}

	}
}

#include "part.moc"

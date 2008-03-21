#include "piece.h"
#include "part.h"
#include "scene.h"

Palapeli::Piece::Piece(const QPixmap& pixmap, Palapeli::Scene* scene, int xIndex, int yIndex, int width, int height, QGraphicsItem* parent)
	: QGraphicsPixmapItem(parent)
	, m_scene(scene)
	, m_part(0)
	, m_xIndex(xIndex)
	, m_yIndex(yIndex)
{
	setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);

	setPixmap(pixmap);
	m_scene->addItem(this);
	QRectF pieceRect = sceneBoundingRect();
	scale(width / pieceRect.width(), height / pieceRect.height());
}

Palapeli::Piece::~Piece()
{
}

void Palapeli::Piece::setPart(Palapeli::Part* part) //friend of Palapeli::Part
{
	//change ownership in internal data structures of Part
	if (m_part != 0)
		m_part->m_pieces.removeAll(this);
	if (part != 0)
		part->m_pieces << this;
	//setup my own relationships
	m_part = part;
	setParentItem(part);
}

Palapeli::Part* Palapeli::Piece::part() const
{
	return m_part;
}

int Palapeli::Piece::xIndex() const
{
	return m_xIndex;
}

int Palapeli::Piece::yIndex() const
{
	return m_yIndex;
}

void Palapeli::Piece::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	m_part->mouseMoveEvent(event);
}

void Palapeli::Piece::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	m_part->mousePressEvent(event);

}

void Palapeli::Piece::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	m_part->mouseReleaseEvent(event);

}
#include "piece.moc"

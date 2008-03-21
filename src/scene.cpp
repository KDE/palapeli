#include "scene.h"
#include "part.h"
#include "piece.h"

#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <KDebug>

Palapeli::Scene::Scene()
	: QGraphicsScene()
	, m_xPieces(0)
	, m_yPieces(0)
	, m_pieces(0)
{
	setSceneRect(0, 0, 2000, 2000);
}

Palapeli::Scene::~Scene()
{
	for (int x = 0; x < m_xPieces; ++x)
		delete[] m_pieces[x];
	delete[] m_pieces;
	foreach (Palapeli::Part *part, m_parts)
		delete part;
}

void Palapeli::Scene::loadImage(const QString &fileName, int xPieces, int yPieces)
{
	m_xPieces = xPieces;
	m_yPieces = yPieces;
	QImage image(fileName);
	int width = image.width(), height = image.height();
	int pieceWidth = width / xPieces, pieceHeight = height / yPieces;
	m_pieces = new Palapeli::Piece**[xPieces];
	for (int x = 0; x < xPieces; ++x)
	{
		m_pieces[x] = new Palapeli::Piece*[yPieces];
		for (int y = 0; y < yPieces; ++y)
		{
			QPixmap pix(pieceWidth, pieceHeight);
			QPainter painter(&pix);
			painter.drawImage(QPoint(0, 0), image, QRect(x * pieceWidth, y * pieceHeight, pieceWidth, pieceHeight));
			painter.end();
			m_pieces[x][y] = new Palapeli::Piece(pix, this, x, y, pieceWidth, pieceHeight);
			Palapeli::Part* part = new Palapeli::Part(m_pieces[x][y], this);
			addItem(part);
			part->setPos(qrand() % 1000, qrand() % 1000); 
			m_parts << part;
		}
	}
}

Palapeli::Piece* Palapeli::Scene::topNeighbor(int xIndex, int yIndex)
{
	return (yIndex == 0) ? 0 : m_pieces[xIndex][yIndex - 1]; 
}

Palapeli::Piece* Palapeli::Scene::bottomNeighbor(int xIndex, int yIndex)
{
	return (yIndex == m_yPieces - 1) ? 0 : m_pieces[xIndex][yIndex + 1]; 
}

Palapeli::Piece* Palapeli::Scene::leftNeighbor(int xIndex, int yIndex)
{
	return (xIndex == 0) ? 0 : m_pieces[xIndex - 1][yIndex]; 
}

Palapeli::Piece* Palapeli::Scene::rightNeighbor(int xIndex, int yIndex) 
{
	return (xIndex == m_xPieces - 1) ? 0 : m_pieces[xIndex + 1][yIndex]; 
}

void Palapeli::Scene::combineParts(Palapeli::Part* part1, Palapeli::Part* part2, qreal dx, qreal dy) //friend of Palapeli::Part
{
	QPointF pos1 = part1->pos();
	QPointF pos2 = part2->pos();
	dx += pos2.x() - pos1.x();
	dy += pos2.y() - pos1.y();
	if (!m_parts.contains(part2))
		return;
	foreach (Palapeli::Piece *piece, part2->m_pieces)
	{
		piece->setPart(part1);
		piece->moveBy(dx, dy);
	}
	m_parts.removeAll(part2);
	delete part2;
}

#include "scene.moc"

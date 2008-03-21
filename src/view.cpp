#include "view.h"
#include "scene.h"

Palapeli::View::View(QWidget* parent)
	: QGraphicsView(parent)
	, m_scene(0)
{
}

Palapeli::View::~View()
{
	delete m_scene;
}

void Palapeli::View::startGame(const QString &fileName, int xPieces, int yPieces)
{
	delete m_scene;
	m_scene = new Palapeli::Scene;
	m_scene->loadImage(fileName, xPieces, yPieces);
	setScene(m_scene);
}

#include "view.moc"

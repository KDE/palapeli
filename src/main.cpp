/***************************************************************************
 *   Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
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

#include "engine/part.h"
#include "engine/piece.h"
#include <QGraphicsView>

#include <ctime>
#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>
#include <KGlobal>
#include <KLocale>

int main(int argc, char** argv)
{
	qsrand(time(0));
	KAboutData about("palapeli", 0, ki18nc("The application's name", "Palapeli"), "1.0", ki18n("KDE Jigsaw Puzzle Game"), KAboutData::License_GPL, ki18n("Copyright 2009, Stefan Majewsky"));
	about.addAuthor(ki18n("Stefan Majewsky"), KLocalizedString(), "majewsky@gmx.net", "http://majewsky.wordpress.com");
	KCmdLineArgs::init(argc, argv, &about);

	KApplication app;
	KGlobal::locale()->insertCatalog("libkdegames");

	QGraphicsScene scene;
	QPixmap pixmap1(100, 100);
	pixmap1.fill(Qt::red);
	Palapeli::Piece* piece1 = new Palapeli::Piece(pixmap1, QPoint(0, 0));
	QPixmap pixmap2(200, 100);
	pixmap2.fill(Qt::yellow);
	Palapeli::Piece* piece2 = new Palapeli::Piece(pixmap2, QPoint(100, 0));
	QPixmap pixmap3(100, 200);
	pixmap3.fill(Qt::green);
	Palapeli::Piece* piece3 = new Palapeli::Piece(pixmap3, QPoint(0, 100));
	QPixmap pixmap4(200, 200);
	pixmap4.fill(Qt::blue);
	Palapeli::Piece* piece4 = new Palapeli::Piece(pixmap4, QPoint(100, 100));
	piece1->addNeighbor(piece2);
	piece1->addNeighbor(piece3);
	piece2->addNeighbor(piece1);
	piece2->addNeighbor(piece4);
	piece3->addNeighbor(piece1);
	piece3->addNeighbor(piece4);
	piece4->addNeighbor(piece2);
	piece4->addNeighbor(piece3);

	QList<Palapeli::Piece*> pieces; pieces << piece1 << piece2 << piece3 << piece4;
	foreach (Palapeli::Piece* piece, pieces)
	{
		Palapeli::Part* part = new Palapeli::Part(piece);
		scene.addItem(part);
		part->setPos(qrand() % 300, qrand() % 300);
	}

	QGraphicsView view;
	view.setScene(&scene);
	view.resize(800, 600);
	view.show();

	return app.exec();
}

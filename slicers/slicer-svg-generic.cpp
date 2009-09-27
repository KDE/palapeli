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

//NOTE: This is generic code. It needs the following preprocessor variables and C++ constants:
//  #define CLASSNAME           -> The name of the slicer class (i.e. the class that inherits from Pala::Slicer).
//  const char* PLUGINNAME      -> The name of the plugin (as literal string constant).
//  const QString SHAPENAME     -> The file name component for the base shapes of this SVG slicer.
//See slicer-svg-jigsaw.cpp for an example on how to use this code template.

#include <QPainter>
#include <QSvgRenderer>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KStandardDirs>

K_PLUGIN_FACTORY(SvgSlicerFactory, registerPlugin<CLASSNAME>();)
K_EXPORT_PLUGIN(SvgSlicerFactory(PLUGINNAME))

CLASSNAME::CLASSNAME(QObject* parent, const QVariantList& args)
	: Pala::Slicer(parent, args)
{
	Pala::SlicerProperty* prop;
	prop = new Pala::SlicerProperty(Pala::SlicerProperty::Integer, i18n("Piece count in horizontal direction"));
	prop->setRange(3, 100);
	prop->setDefaultValue(10);
	addProperty("XCount", prop);
	prop = new Pala::SlicerProperty(Pala::SlicerProperty::Integer, i18n("Piece count in vertical direction"));
	prop->setRange(3, 100);
	prop->setDefaultValue(10);
	addProperty("YCount", prop);
}

bool CLASSNAME::run(Pala::SlicerJob* job)
{
	//load renderers
	QList<QSvgRenderer*> shapes; int shapeCount = 0;
	shapes << new QSvgRenderer(KStandardDirs::locate("data", "palapeli/jigsaw-pics/shape-0.svg"));
	const QString maleShapeName = QString::fromLatin1("palapeli/jigsaw-pics/%1-%2-male.svg").arg(SHAPENAME);
	const QString femaleShapeName = QString::fromLatin1("palapeli/jigsaw-pics/%1-%2-female.svg").arg(SHAPENAME);
	for (int i = 1; ; ++i) // attention: i = 1..n because i = 0 is used for the null shape (without plug)
	{
		const QString thisMaleShapeName = KStandardDirs::locate("data", maleShapeName.arg(i));
		const QString thisFemaleShapeName = KStandardDirs::locate("data", femaleShapeName.arg(i));
		if (thisMaleShapeName.isEmpty() || thisFemaleShapeName.isEmpty())
			break;
		else
		{
			++shapeCount;
			shapes << new QSvgRenderer(thisMaleShapeName); //is now at position 2 * i - 1
			shapes << new QSvgRenderer(thisFemaleShapeName); //is now at position 2 * i
		}
	}
	if (shapeCount == 0)
		return false;
	//read job
	//TODO: Does not check input values. Perhaps introduce a check in the Slicer base class?
	const int xCount = job->argument("XCount").toInt();
	const int yCount = job->argument("YCount").toInt();
	const QImage image = job->image();
	//calculate some metrics
	const int width = image.width(), height = image.height();
	const int pieceWidth = width / xCount, pieceHeight = height / yCount;
	const int plugPaddingX = pieceWidth / 2, plugPaddingY = pieceHeight / 2; //see below
	//find plug shape types, based on given seed
	int** horizontalPlugShapeTypes = new int*[xCount];
	int** verticalPlugShapeTypes = new int*[xCount];
	bool** horizontalPlugDirections = new bool*[xCount]; //true: male is left, female is right, plug points to the right
	bool** verticalPlugDirections = new bool*[xCount]; //true: male is above female, plug points down
	for (int x = 0; x < xCount; ++x)
	{
		horizontalPlugShapeTypes[x] = new int[yCount];
		verticalPlugShapeTypes[x] = new int[yCount];
		horizontalPlugDirections[x] = new bool[yCount];
		verticalPlugDirections[x] = new bool[yCount];
		for (int y = 0; y < yCount; ++y)
		{
			//plugs along X axis
			horizontalPlugShapeTypes[x][y] = qrand() % shapeCount + 1; //values: 1 .. shapeCount
			horizontalPlugDirections[x][y] = qrand() % 2;
			//plugs along Y axis
			verticalPlugShapeTypes[x][y] = qrand() % shapeCount + 1;
			verticalPlugDirections[x][y] = qrand() % 2;
		}
	}
	//create pieces
	for (int x = 0; x < xCount; ++x)
	{
		for (int y = 0; y < yCount; ++y)
		{
			//some geometry
			const QRect pieceBaseRect( //piece without plugs
				x * pieceWidth,
				y * pieceHeight,
				pieceWidth,
				pieceHeight
			);
			const QRect pieceRect( //piece with padding space for plugs (will overlap with neighbor piece rects)
				x * pieceWidth - plugPaddingX,
				y * pieceHeight - plugPaddingY,
				pieceWidth + 2 * plugPaddingX,
				pieceHeight + 2 * plugPaddingY
			);
			const QRect maskBaseRect( //the part of the mask that maps to pieceBaseRect
				plugPaddingX,
				plugPaddingY,
				pieceWidth,
				pieceHeight
			);
			const QRect maskRect( //the whole mask; maps to pieceRect
				0,
				0,
				pieceWidth + 2 * plugPaddingX,
				pieceHeight + 2 * plugPaddingY
			);
			//create the mask
			QImage mask(maskRect.size(), QImage::Format_ARGB32_Premultiplied);
			mask.fill(0x00000000); //fully transparent color
			QPainter painter(&mask);
			painter.setPen(Qt::NoPen);
			painter.setBrush(Qt::black);
			//left plug
			painter.save();
			if (x == 0)
			{
				//nothing to plug at the left side - use null shape which points to the right originally
				painter.translate(maskRect.center());
				painter.rotate(180);
				painter.translate(-maskRect.center());
				shapes[0]->render(&painter, maskRect);
			}
			else if (horizontalPlugDirections[x - 1][y]) //here is female plug which points to the left already
			{
				const int shapeType = horizontalPlugShapeTypes[x - 1][y];
				shapes[2 * shapeType]->render(&painter, maskRect);
			}
			else //here is male plug which points to the right originally
			{
				painter.translate(maskRect.center());
				painter.rotate(180);
				painter.translate(-maskRect.center());
				const int shapeType = horizontalPlugShapeTypes[x - 1][y];
				shapes[2 * shapeType - 1]->render(&painter, maskRect);
			}
			painter.restore();
			//right plug
			painter.save();
			if (x == xCount - 1)
			{
				//nothing to plug at the right side - use null shape which points to the right already
				shapes[0]->render(&painter, maskRect);
			}
			else if (horizontalPlugDirections[x][y]) //here is male plug which points to the right already
			{
				const int shapeType = horizontalPlugShapeTypes[x][y];
				shapes[2 * shapeType - 1]->render(&painter, maskRect);
			}
			else //here is female plug which points to the left originally
			{
				painter.translate(maskRect.center());
				painter.rotate(180);
				painter.translate(-maskRect.center());
				const int shapeType = horizontalPlugShapeTypes[x][y];
				shapes[2 * shapeType]->render(&painter, maskRect);
			}
			painter.restore();
			//top plug
			painter.save();
			if (y == 0)
			{
				//nothing to plug at the top side - use null shape which points to the right originally
				painter.translate(maskRect.center());
				painter.rotate(-90);
				painter.scale(qreal(maskRect.height()) / maskRect.width(), qreal(maskRect.width()) / maskRect.height());
				painter.translate(-maskRect.center());
				shapes[0]->render(&painter, maskRect);
			}
			else if (verticalPlugDirections[x][y - 1]) //here is female plug which points to the left originally
			{
				painter.translate(maskRect.center());
				painter.rotate(90);
				painter.scale(qreal(maskRect.height()) / maskRect.width(), qreal(maskRect.width()) / maskRect.height());
				painter.translate(-maskRect.center());
				const int shapeType = verticalPlugShapeTypes[x][y - 1];
				shapes[2 * shapeType]->render(&painter, maskRect);
			}
			else //here is male plug which points to the right originally
			{
				painter.translate(maskRect.center());
				painter.rotate(-90);
				painter.scale(qreal(maskRect.height()) / maskRect.width(), qreal(maskRect.width()) / maskRect.height());
				painter.translate(-maskRect.center());
				const int shapeType = verticalPlugShapeTypes[x][y - 1];
				shapes[2 * shapeType - 1]->render(&painter, maskRect);
			}
			painter.restore();
			//bottom plug
			painter.save();
			if (y == yCount - 1)
			{
				//nothing to plug at the bottom side - use null shape which points to the right originally
				painter.translate(maskRect.center());
				painter.rotate(90);
				painter.scale(qreal(maskRect.height()) / maskRect.width(), qreal(maskRect.width()) / maskRect.height());
				painter.translate(-maskRect.center());
				shapes[0]->render(&painter, maskRect);
			}
			else if (verticalPlugDirections[x][y]) //here is female plug which points to the left originally
			{
				painter.translate(maskRect.center());
				painter.rotate(90);
				painter.scale(qreal(maskRect.height()) / maskRect.width(), qreal(maskRect.width()) / maskRect.height());
				painter.translate(-maskRect.center());
				const int shapeType = verticalPlugShapeTypes[x][y];
				shapes[2 * shapeType - 1]->render(&painter, maskRect);
			}
			else //here is male plug which points to the right originally
			{
				painter.translate(maskRect.center());
				painter.rotate(-90);
				painter.scale(qreal(maskRect.height()) / maskRect.width(), qreal(maskRect.width()) / maskRect.height());
				painter.translate(-maskRect.center());
				const int shapeType = verticalPlugShapeTypes[x][y];
				shapes[2 * shapeType]->render(&painter, maskRect);
			}
			painter.restore();
			//done creating the mask
			painter.end();
			job->addPieceFromMask(x + y * xCount, mask, pieceRect.topLeft());
		}
	}
	//create relations
	for (int x = 0; x < xCount; ++x)
	{
		for (int y = 0; y < yCount; ++y)
		{
			//along X axis (pointing left)
			if (x != 0)
				job->addRelation(x + y * xCount, (x - 1) + y * xCount);
			//along Y axis (pointing up)
			if (y != 0)
				job->addRelation(x + y * xCount, x + (y - 1) * xCount);
		}
	}
	//cleanup
	qDeleteAll(shapes);
	for (int x = 0; x < xCount - 1; ++x)
	{
		delete[] horizontalPlugShapeTypes[x];
		delete[] verticalPlugShapeTypes[x];
		delete[] horizontalPlugDirections[x];
		delete[] verticalPlugDirections[x];
	}
	delete[] horizontalPlugShapeTypes;
	delete[] verticalPlugShapeTypes;
	delete[] horizontalPlugDirections;
	delete[] verticalPlugDirections;
	return true;
}


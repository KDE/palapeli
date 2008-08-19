/***************************************************************************
 *   Copyright (C) 2008 Stefan Majewsky <majewsky@gmx.net>
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

#include "pattern-jigsaw.h"

#include <ctime>
#include <QImage>
#include <QPainter>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KStandardDirs>
#include <KSvgRenderer>

K_PLUGIN_FACTORY(JigsawPatternFactory, registerPlugin<Palapeli::JigsawPatternPlugin>();)
K_EXPORT_PLUGIN(JigsawPatternFactory("palapeli_jigsawpattern"))

const QString NullShapeName("palapeli/jigsaw-pics/shape-0.svg");
QRegExp ShapeNameExtractor("palapeli/jigsaw-pics/(.*)-\\d*-(fe)?male.svg$");
const QString MaleShapeName("palapeli/jigsaw-pics/%1-%2-male.svg");
const QString FemaleShapeName("palapeli/jigsaw-pics/%1-%2-female.svg");

//BEGIN Palapeli::JigsawPattern

Palapeli::JigsawPattern::JigsawPattern(int xCount, int yCount, const QString& theme, int seed)
	: Palapeli::Pattern()
	, m_xCount(qMax(1, xCount))
	, m_yCount(qMax(1, yCount))
	, m_shapeCount(0)
	, m_seed(seed)
{
	//load renderers
	KStandardDirs dirs;
	m_shapes << new KSvgRenderer(dirs.locate("data", NullShapeName));
	const QString maleShapeName = MaleShapeName.arg(theme);
	const QString femaleShapeName = FemaleShapeName.arg(theme);
	for (int i = 1; ; ++i) // attention: i = 1..n because i = 0 is used for the null shape (without plug)
	{
		const QString thisMaleShapeName = dirs.locate("data", maleShapeName.arg(i));
		const QString thisFemaleShapeName = dirs.locate("data", femaleShapeName.arg(i));
		if (thisMaleShapeName.isEmpty() || thisFemaleShapeName.isEmpty())
			break;
		else
		{
			++m_shapeCount;
			m_shapes << new KSvgRenderer(thisMaleShapeName); //is now at position 2 * i - 1
			m_shapes << new KSvgRenderer(thisFemaleShapeName); //is now at position 2 * i
		}
	}
}

Palapeli::JigsawPattern::~JigsawPattern()
{
	foreach (KSvgRenderer* shape, m_shapes)
		delete shape;
}

void Palapeli::JigsawPattern::doSlice(const QImage& image)
{
	reportPieceCount(m_xCount * m_yCount);
	const int width = image.width(), height = image.height();
	const int pieceWidth = width / m_xCount, pieceHeight = height / m_yCount;
	const int plugPaddingX = pieceWidth / 4, plugPaddingY = pieceHeight / 4; //see below
	//find plug shape types, based on given seed
	qsrand(m_seed);
	int** horizontalPlugShapeTypes = new int*[m_xCount];
	int** verticalPlugShapeTypes = new int*[m_xCount];
	bool** horizontalPlugDirections = new bool*[m_xCount]; //true: male is left, female is right, plug points to the right
	bool** verticalPlugDirections = new bool*[m_xCount]; //true: male is above female, plug points down
	for (int x = 0; x < m_xCount; ++x)
	{
		horizontalPlugShapeTypes[x] = new int[m_yCount];
		verticalPlugShapeTypes[x] = new int[m_yCount];
		horizontalPlugDirections[x] = new bool[m_yCount];
		verticalPlugDirections[x] = new bool[m_yCount];
		for (int y = 0; y < m_yCount; ++y)
		{
			//plugs along X axis
			horizontalPlugShapeTypes[x][y] = qrand() % m_shapeCount + 1; //values: 1 .. m_shapeCount
			horizontalPlugDirections[x][y] = qrand() % 2;
			//plugs along Y axis
			verticalPlugShapeTypes[x][y] = qrand() % m_shapeCount + 1;
			verticalPlugDirections[x][y] = qrand() % 2;
		}
	}
	//make pieces
	for (int x = 0; x < m_xCount; ++x)
	{
		for (int y = 0; y < m_yCount; ++y)
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
				m_shapes[0]->render(&painter, maskRect);
			}
			else if (horizontalPlugDirections[x - 1][y]) //here is female plug which points to the left already
			{
				const int shapeType = horizontalPlugShapeTypes[x - 1][y];
				m_shapes[2 * shapeType]->render(&painter, maskRect);
			}
			else //here is male plug which points to the right originally
			{
				painter.translate(maskRect.center());
				painter.rotate(180);
				painter.translate(-maskRect.center());
				const int shapeType = horizontalPlugShapeTypes[x - 1][y];
				m_shapes[2 * shapeType - 1]->render(&painter, maskRect);
			}
			painter.restore();
			//right plug
			painter.save();
			if (x == m_xCount - 1)
			{
				//nothing to plug at the right side - use null shape which points to the right already
				m_shapes[0]->render(&painter, maskRect);
			}
			else if (horizontalPlugDirections[x][y]) //here is male plug which points to the right already
			{
				const int shapeType = horizontalPlugShapeTypes[x][y];
				m_shapes[2 * shapeType - 1]->render(&painter, maskRect);
			}
			else //here is female plug which points to the left originally
			{
				painter.translate(maskRect.center());
				painter.rotate(180);
				painter.translate(-maskRect.center());
				const int shapeType = horizontalPlugShapeTypes[x][y];
				m_shapes[2 * shapeType]->render(&painter, maskRect);
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
				m_shapes[0]->render(&painter, maskRect);
			}
			else if (verticalPlugDirections[x][y - 1]) //here is female plug which points to the left originally
			{
				painter.translate(maskRect.center());
				painter.rotate(90);
				painter.scale(qreal(maskRect.height()) / maskRect.width(), qreal(maskRect.width()) / maskRect.height());
				painter.translate(-maskRect.center());
				const int shapeType = verticalPlugShapeTypes[x][y - 1];
				m_shapes[2 * shapeType]->render(&painter, maskRect);
			}
			else //here is male plug which points to the right originally
			{
				painter.translate(maskRect.center());
				painter.rotate(-90);
				painter.scale(qreal(maskRect.height()) / maskRect.width(), qreal(maskRect.width()) / maskRect.height());
				painter.translate(-maskRect.center());
				const int shapeType = verticalPlugShapeTypes[x][y - 1];
				m_shapes[2 * shapeType - 1]->render(&painter, maskRect);
			}
			painter.restore();
			//bottom plug
			painter.save();
			if (y == m_yCount - 1)
			{
				//nothing to plug at the bottom side - use null shape which points to the right originally
				painter.translate(maskRect.center());
				painter.rotate(90);
				painter.scale(qreal(maskRect.height()) / maskRect.width(), qreal(maskRect.width()) / maskRect.height());
				painter.translate(-maskRect.center());
				m_shapes[0]->render(&painter, maskRect);
			}
			else if (verticalPlugDirections[x][y]) //here is female plug which points to the left originally
			{
				painter.translate(maskRect.center());
				painter.rotate(90);
				painter.scale(qreal(maskRect.height()) / maskRect.width(), qreal(maskRect.width()) / maskRect.height());
				painter.translate(-maskRect.center());
				const int shapeType = verticalPlugShapeTypes[x][y];
				m_shapes[2 * shapeType - 1]->render(&painter, maskRect);
			}
			else //here is male plug which points to the right originally
			{
				painter.translate(maskRect.center());
				painter.rotate(-90);
				painter.scale(qreal(maskRect.height()) / maskRect.width(), qreal(maskRect.width()) / maskRect.height());
				painter.translate(-maskRect.center());
				const int shapeType = verticalPlugShapeTypes[x][y];
				m_shapes[2 * shapeType]->render(&painter, maskRect);
			}
			painter.restore();
			//done creating the mask
			painter.end();
			addPiece(image.copy(pieceRect), mask, pieceRect);
		}
	}
	//build relationships between pieces
	for (int x = 0; x < m_xCount; ++x)
	{
		for (int y = 0; y < m_yCount; ++y)
		{
			//along X axis (pointing left)
			if (x != 0)
				addRelation(x * m_yCount + y, (x - 1) * m_yCount + y);
			//along Y axis (pointing up)
			if (y != 0)
				addRelation(x * m_yCount + y, x * m_yCount + (y - 1));
		}
	}
	//do some cleanup
	for (int x = 0; x < m_xCount - 1; ++x)
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
	//randomize to avoid any abstruse side effects
	qsrand(time(0));
}

//END Palapeli::JigsawPattern

//BEGIN Palapeli::JigsawPatternConfiguration

Palapeli::JigsawPatternConfiguration::JigsawPatternConfiguration(const QString& themeName, const QString& pluginName, const QString& displayName)
	: Palapeli::PatternConfiguration(pluginName, displayName)
	, m_seed(time(0))
	, m_themeName(themeName)
{
	//add properties
	addProperty("XCount", Palapeli::PatternConfiguration::Integer, i18n("Piece count in horizontal direction:"));
	addProperty("YCount", Palapeli::PatternConfiguration::Integer, i18n("Piece count in vertical direction:"));
	//set parameters (minimum and maximum in this case)
	QVariantList params; params << 3 << 100;
	addPropertyParameters("XCount", params);
	addPropertyParameters("YCount", params);
	//set default values
	setProperty("XCount", 10);
	setProperty("YCount", 10);
}

Palapeli::JigsawPatternConfiguration::~JigsawPatternConfiguration()
{
}

Palapeli::Pattern* Palapeli::JigsawPatternConfiguration::createPattern() const
{
	return new Palapeli::JigsawPattern(property("XCount").toInt(), property("YCount").toInt(), m_themeName, m_seed);
}

void Palapeli::JigsawPatternConfiguration::readCustomArguments(KConfigGroup* config)
{
	m_seed = config->readEntry("Seed", 0);
}

void Palapeli::JigsawPatternConfiguration::writeCustomArguments(KConfigGroup* config) const
{
	config->writeEntry("Seed", m_seed);
}

//END Palapeli::JigsawPatternConfiguration

//BEGIN Palapeli::JigsawPatternPlugin

#include <KDebug>

Palapeli::JigsawPatternPlugin::JigsawPatternPlugin(QObject* parent, const QVariantList& args)
	: Palapeli::PatternPlugin(parent, args)
{
	//get possible shape themes
	const QStringList shapeFiles = KStandardDirs().findAllResources("data", MaleShapeName.arg("*").arg("1"), KStandardDirs::NoDuplicates);
	foreach (const QString& shapeFile, shapeFiles)
	{
		ShapeNameExtractor.indexIn(shapeFile);
		const QString themeName = ShapeNameExtractor.cap(1);
		if (!themeName.isEmpty())
			m_themeNames << themeName;
	}
}

Palapeli::JigsawPatternPlugin::~JigsawPatternPlugin()
{
}

QList<Palapeli::PatternConfiguration*> Palapeli::JigsawPatternPlugin::createInstances() const
{
	QList<Palapeli::PatternConfiguration*> list;
	foreach (const QString& themeName, m_themeNames)
		list << new Palapeli::JigsawPatternConfiguration(themeName, pluginName().arg(themeName), displayName().arg(themeName));
	return list;
}

//END Palapeli::JigsawPatternPlugin

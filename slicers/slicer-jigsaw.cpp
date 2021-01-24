/*
    SPDX-FileCopyrightText: 2009 Stefan Majewsky <majewsky@gmx.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "slicer-jigsaw.h"

#include <cmath>
#include <QPainter>
#include <QPainterPath>
#include <KPluginFactory>
#include <QRandomGenerator>

//BEGIN utility functions

qreal myrand(qreal min, qreal max)
{
        const qreal randNum = qreal(QRandomGenerator::global()->bounded(10000)) / 10000; //a quite random number between 0 and 1
	return randNum * (max - min) + min;
}

inline qreal operator*(const QPointF& a, const QPointF& b)
{
	return a.x() * b.x() + a.y() * b.y();
}

//NOTE: The lines in the following methods are always directed clockwise, which is an important property!

QLineF topSide(const QRect& rect)
{
	return QLineF(rect.topLeft(), rect.topRight());
}

QLineF rightSide(const QRect& rect)
{
	return QLineF(rect.topRight(), rect.bottomRight());
}

QLineF bottomSide(const QRect& rect)
{
	return QLineF(rect.bottomRight(), rect.bottomLeft());
}

QLineF leftSide(const QRect& rect)
{
	return QLineF(rect.bottomLeft(), rect.topLeft());
}

//END utility functions

/*static*/ JigsawPlugParams JigsawPlugParams::createRandomParams()
{
	JigsawPlugParams result;
	result.plugPosition = myrand(0.35, 0.65);
	const qreal maxPlugLength = 0.4 - 0.88 * qAbs(0.5 - result.plugPosition);
	result.plugLength = myrand(0.75, 1.0) * maxPlugLength;
	result.plugWidth = myrand(0.18, 0.38);
	const qreal minDistortion1 = 0.75 * (0.7 + result.plugWidth);
	result.distortion1 = myrand(minDistortion1, minDistortion1 * 1.1);
	result.distortion2 = myrand(0.4, 1.0);
	result.baseHeight = myrand(0.0, 0.2);
	result.baseDistortion = myrand(0.0, 1.0);
	return result;
}

JigsawPlugParams JigsawPlugParams::mirrored()
{
	JigsawPlugParams result(*this);
	result.plugPosition = 1 - result.plugPosition;
	return result;
}

K_PLUGIN_CLASS_WITH_JSON(JigsawSlicer, "palapeli_jigsawslicer.json")

JigsawSlicer::JigsawSlicer(QObject* parent, const QVariantList& args)
	: Pala::Slicer(parent, args)
	, Pala::SimpleGridPropertySet(this)
{
}

bool JigsawSlicer::run(Pala::SlicerJob* job)
{
	//read job
	const QSize pieceCount = Pala::SimpleGridPropertySet::pieceCount(job);
	const int xCount = pieceCount.width();
	const int yCount = pieceCount.height();
	const QImage image = job->image();
	//calculate some metrics
	const int width = image.width(), height = image.height();
	const int pieceWidth = width / xCount, pieceHeight = height / yCount;
	const int plugPaddingX = pieceWidth / 2, plugPaddingY = pieceHeight / 2; //see below
	//find plug shape types
	JigsawPlugParams** horizontalPlugParams = new JigsawPlugParams*[xCount];
	JigsawPlugParams** verticalPlugParams = new JigsawPlugParams*[xCount];
	int** horizontalPlugDirections = new int*[xCount]; //+1: male is left, female is right, plug points to the right (-1 is the opposite direction)
	int** verticalPlugDirections = new int*[xCount]; //true: male is above female, plug points down
        auto *generator = QRandomGenerator::global();
	for (int x = 0; x < xCount; ++x)
	{
		horizontalPlugParams[x] = new JigsawPlugParams[yCount];
		verticalPlugParams[x] = new JigsawPlugParams[yCount];
		horizontalPlugDirections[x] = new int[yCount];
		verticalPlugDirections[x] = new int[yCount];
		for (int y = 0; y < yCount; ++y)
		{
			//plugs along X axis
			horizontalPlugParams[x][y] = JigsawPlugParams::createRandomParams();
                        horizontalPlugDirections[x][y] = (generator->bounded(2)) ? 1 : -1;
			//plugs along Y axis
			verticalPlugParams[x][y] = JigsawPlugParams::createRandomParams();
                        verticalPlugDirections[x][y] = (generator->bounded(2)) ? 1 : -1;
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
			QRect pieceRect( //piece with padding space for plugs (will overlap with neighbor piece rects)
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
			//create the mask path
			QPainterPath path;
			path.moveTo(maskBaseRect.topLeft());
			//top plug
			if (y == 0)
				path.lineTo(maskBaseRect.topRight());
			else
				addPlugToPath(path, maskBaseRect.height(), topSide(maskBaseRect), QPointF(0, verticalPlugDirections[x][y - 1]), verticalPlugParams[x][y - 1]);
			//right plug
			if (x == xCount - 1)
				path.lineTo(maskBaseRect.bottomRight());
			else
				addPlugToPath(path, maskBaseRect.width(), rightSide(maskBaseRect), QPointF(horizontalPlugDirections[x][y], 0), horizontalPlugParams[x][y].mirrored());
			//bottom plug
			if (y == yCount - 1)
				path.lineTo(maskBaseRect.bottomLeft());
			else
				addPlugToPath(path, maskBaseRect.height(), bottomSide(maskBaseRect), QPointF(0, verticalPlugDirections[x][y]), verticalPlugParams[x][y].mirrored());
			//left plug
			if (x == 0)
				path.lineTo(maskBaseRect.topLeft());
			else
				addPlugToPath(path, maskBaseRect.width(), leftSide(maskBaseRect), QPointF(horizontalPlugDirections[x - 1][y], 0), horizontalPlugParams[x - 1][y]);
			//determine the required size of the mask
			path.closeSubpath();
			const QRect newMaskRect = path.boundingRect().toAlignedRect();
			pieceRect.adjust(
				newMaskRect.left() - maskRect.left(),
				newMaskRect.top() - maskRect.top(),
				newMaskRect.right() - maskRect.right(),
				newMaskRect.bottom() - maskRect.bottom()
			);
			//create the mask
			QImage mask(newMaskRect.size(), QImage::Format_ARGB32_Premultiplied);
			mask.fill(0x00000000); //fully transparent color
			QPainter painter(&mask);
			painter.translate(maskRect.topLeft() - newMaskRect.topLeft());
			painter.setPen(QPen(Qt::black, 1.5)); //we explicitly use a pen stroke in order to let the pieces overlap a bit (which reduces rendering glitches at the edges where puzzle pieces touch)
			painter.setBrush(Qt::black);
			painter.setRenderHint(QPainter::Antialiasing);
			painter.drawPath(path);
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
	for (int x = 0; x < xCount - 1; ++x)
	{
		delete[] horizontalPlugParams[x];
		delete[] verticalPlugParams[x];
		delete[] horizontalPlugDirections[x];
		delete[] verticalPlugDirections[x];
	}
	delete[] horizontalPlugParams;
	delete[] verticalPlugParams;
	delete[] horizontalPlugDirections;
	delete[] verticalPlugDirections;
	return true;
}

void JigsawSlicer::addPlugToPath(QPainterPath& path, qreal plugNormLength, const QLineF& line, const QPointF& plugDirection, const JigsawPlugParams& params)
{
	//Naming convention: The path runs through five points (p1 through p5).
	//pNbase is the projection of pN to the line between p1 and p5.
	//tN is the parameter of pNbase on the line between p1 and p5 (with t1 = 0 and t5 = 1).
	//qN is the control point of pN on the cubic between p{N-1} and pN.
	//rN is the control point of pN on the cubic between pN and p{N+1}.
	const QPointF p1 = line.p1(), p5 = line.p2();
	const QPointF growthDirection = plugDirection / sqrt(plugDirection * plugDirection);
	//const qreal sizeFactor = line.length();
	//const QPointF growthVector = growthDirection * sizeFactor;
	const QPointF plugVector = params.plugLength * plugNormLength * growthDirection;
	//calculate points p2, p3, p4
	const qreal t3 = params.plugPosition;
	const QPointF p3base = (1.0 - t3) * p1 + t3 * p5;
	const QPointF p3 = p3base + plugVector;
	const qreal t2 = params.plugPosition - params.plugWidth / 2.0;
	const QPointF p2base = (1.0 - t2) * p1 + t2 * p5;
	const QPointF p2 = p2base + params.baseHeight * plugVector;
	const qreal t4 = params.plugPosition + params.plugWidth / 2.0;
	const QPointF p4base = (1.0 - t4) * p1 + t4 * p5;
	const QPointF p4 = p4base + params.baseHeight * plugVector;
	//calculate control points
	const QPointF r1 = p1;
	const qreal tr2 = params.distortion1 * t2;
	const QPointF r2base = (1.0 - tr2) * p1 + tr2 * p5;
	const QPointF r2 = r2base + params.distortion2 * plugVector;
	const QPointF q2 = p2 + params.baseDistortion * (p2 - r2);
	const QPointF q3 = p3 + (p2base - p3base);
	const QPointF r3 = p3 + (p4base - p3base);
	const qreal tq4 = 1 - (params.distortion1 * (1 - t4));
	const QPointF q4base = (1.0 - tq4) * p1 + tq4 * p5;
	const QPointF q4 = q4base + params.distortion2 * plugVector;
	const QPointF r4 = p4 + params.baseDistortion * (p4 - q4);
	const QPointF q5 = p5;
	//construct path
	path.lineTo(p1);
	path.cubicTo(r1, q2, p2);
	path.cubicTo(r2, q3, p3);
	path.cubicTo(r3, q4, p4);
	path.cubicTo(r4, q5, p5);
}

#include "slicer-jigsaw.moc"

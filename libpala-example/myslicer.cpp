/***************************************************************************
 * Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ***************************************************************************/

#include "myslicer.h"

#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginLoader>

K_PLUGIN_FACTORY(MySlicerFactory, registerPlugin<MySlicer>();)
K_EXPORT_PLUGIN(MySlicerFactory("myslicer"))

MySlicer::MySlicer(QObject* parent, const QVariantList& args)
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

bool MySlicer::run(Pala::SlicerJob* job)
{
    //read job
    const int xCount = job->argument("XCount").toInt();
    const int yCount = job->argument("YCount").toInt();
    const QImage image = job->image();
    //calculate some metrics
    const int pieceWidth = image.width() / xCount;
    const int pieceHeight = image.height() / yCount;
    const QSize pieceSize(pieceWidth, pieceHeight);
    //create pieces
    for (int x = 0; x < xCount; ++x)
    {
        for (int y = 0; y < yCount; ++y)
        {
            //calculate more metrics
            const QPoint offset(x * pieceWidth, y * pieceHeight);
            const QRect pieceBounds(offset, pieceSize);
            //copy image part to piece
            const QImage pieceImage = image.copy(pieceBounds);
            job->addPiece(x + y * xCount, pieceImage, offset);
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
    return true;
}

#include "myslicer.moc"

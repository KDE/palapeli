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

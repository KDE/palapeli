#ifndef MYSLICER_H
#define MYSLICER_H

#include <Pala/Slicer>
#include <Pala/SlicerJob>
#include <Pala/SlicerProperty>

class MySlicer : public Pala::Slicer
{
    Q_OBJECT
    public:
        MySlicer(QObject* parent = 0, const QVariantList& args = QVariantList());
        virtual bool run(Pala::SlicerJob* job);
};

#endif // MYSLICER_H

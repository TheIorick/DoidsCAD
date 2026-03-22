#ifndef BUILDRESULT_H
#define BUILDRESULT_H

#include <TopoDS_Shape.hxx>

#include <QString>

struct BuildResult
{
    bool success = false;
    TopoDS_Shape shape;
    QString description;
    QString errorMessage;
};

#endif // BUILDRESULT_H

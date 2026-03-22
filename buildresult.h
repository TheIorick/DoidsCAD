#ifndef BUILDRESULT_H
#define BUILDRESULT_H

#include <TopoDS_Shape.hxx>

#include <QString>
#include <QVector>

struct OperationBuildShape
{
    int operationId = 0;
    TopoDS_Shape shape;
};

struct BuildResult
{
    bool success = false;
    TopoDS_Shape shape;
    QString description;
    QString errorMessage;
    QVector<OperationBuildShape> operationShapes;
};

#endif // BUILDRESULT_H

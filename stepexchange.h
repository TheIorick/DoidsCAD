#ifndef STEPEXCHANGE_H
#define STEPEXCHANGE_H

#include <TopoDS_Shape.hxx>

#include <QString>

namespace StepExchange
{
bool importStep(const QString &filePath, TopoDS_Shape &shape, QString *errorMessage = nullptr);
bool exportStep(const QString &filePath, const TopoDS_Shape &shape, QString *errorMessage = nullptr);
}

#endif // STEPEXCHANGE_H

#include "stepexchange.h"

#include <IFSelect_ReturnStatus.hxx>
#include <Interface_Static.hxx>
#include <STEPControl_Reader.hxx>
#include <STEPControl_Writer.hxx>
#include <Standard_Failure.hxx>

#include <QObject>

namespace StepExchange
{
bool importStep(const QString &filePath, TopoDS_Shape &shape, QString *errorMessage)
{
    STEPControl_Reader reader;
    const IFSelect_ReturnStatus readStatus = reader.ReadFile(filePath.toStdString().c_str());

    if (readStatus != IFSelect_RetDone)
    {
        if (errorMessage != nullptr)
            *errorMessage = QObject::tr("OpenCascade could not read the STEP file.");

        return false;
    }

    const Standard_Integer roots = reader.TransferRoots();
    if (roots <= 0)
    {
        if (errorMessage != nullptr)
            *errorMessage = QObject::tr("The STEP file does not contain transferable shapes.");

        return false;
    }

    shape = reader.OneShape();
    if (shape.IsNull())
    {
        if (errorMessage != nullptr)
            *errorMessage = QObject::tr("The imported STEP model is empty.");

        return false;
    }

    return true;
}

bool exportStep(const QString &filePath, const TopoDS_Shape &shape, QString *errorMessage)
{
    if (shape.IsNull())
    {
        if (errorMessage != nullptr)
            *errorMessage = QObject::tr("There is no shape to export.");

        return false;
    }

    try
    {
        STEPControl_Writer writer;
        Interface_Static::SetCVal("write.step.schema", "AP203");

        const IFSelect_ReturnStatus transferStatus = writer.Transfer(shape, STEPControl_AsIs);
        if (transferStatus != IFSelect_RetDone)
        {
            if (errorMessage != nullptr)
                *errorMessage = QObject::tr("OpenCascade could not prepare the shape for STEP export.");

            return false;
        }

        const IFSelect_ReturnStatus writeStatus = writer.Write(filePath.toStdString().c_str());
        if (writeStatus != IFSelect_RetDone)
        {
            if (errorMessage != nullptr)
                *errorMessage = QObject::tr("OpenCascade could not write the STEP file.");

            return false;
        }
    }
    catch (const Standard_Failure &failure)
    {
        if (errorMessage != nullptr)
            *errorMessage = QString::fromLocal8Bit(failure.GetMessageString());

        return false;
    }

    return true;
}
}

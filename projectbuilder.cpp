#include "projectbuilder.h"

#include <BRepPrimAPI_MakeBox.hxx>

namespace
{
double parameterValue(const OperationEntry &operation, const QString &name, const double defaultValue)
{
    for (const OperationParameter &parameter : operation.parameters)
    {
        if (parameter.name == name)
            return parameter.value.toDouble();
    }

    return defaultValue;
}

QString parameterText(const OperationEntry &operation, const QString &name)
{
    for (const OperationParameter &parameter : operation.parameters)
    {
        if (parameter.name == name)
            return parameter.value.toString();
    }

    return QString();
}
}

BuildResult ProjectBuilder::build(const ProjectModel &projectModel, const TopoDS_Shape &importedShapeSnapshot)
{
    BuildResult result;

    if (projectModel.operations().isEmpty())
    {
        result.errorMessage = QStringLiteral("Project does not contain any operations.");
        return result;
    }

    const OperationEntry &lastOperation = projectModel.operations().constLast();
    if (lastOperation.type == QStringLiteral("box"))
    {
        const double length = parameterValue(lastOperation, QStringLiteral("Length"), 120.0);
        const double width = parameterValue(lastOperation, QStringLiteral("Width"), 80.0);
        const double height = parameterValue(lastOperation, QStringLiteral("Height"), 60.0);

        result.shape = BRepPrimAPI_MakeBox(length, width, height).Shape();
        result.description = lastOperation.label;
        result.success = !result.shape.IsNull();
        return result;
    }

    if (lastOperation.type == QStringLiteral("import_step"))
    {
        result.shape = importedShapeSnapshot;
        result.description = parameterText(lastOperation, QStringLiteral("Source"));
        result.success = !result.shape.IsNull();

        if (!result.success)
            result.errorMessage = QStringLiteral("Imported STEP snapshot is not available for rebuild.");

        return result;
    }

    result.errorMessage = QStringLiteral("Unsupported operation type: %1").arg(lastOperation.type);
    return result;
}

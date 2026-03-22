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

    for (const OperationEntry &operation : projectModel.operations())
    {
        TopoDS_Shape operationShape;

        if (operation.type == QStringLiteral("box"))
        {
            const double length = parameterValue(operation, QStringLiteral("Length"), 120.0);
            const double width = parameterValue(operation, QStringLiteral("Width"), 80.0);
            const double height = parameterValue(operation, QStringLiteral("Height"), 60.0);
            operationShape = BRepPrimAPI_MakeBox(length, width, height).Shape();
            result.description = operation.label;
        }
        else if (operation.type == QStringLiteral("import_step"))
        {
            operationShape = importedShapeSnapshot;
            result.description = parameterText(operation, QStringLiteral("Source"));
        }
        else
        {
            result.errorMessage = QStringLiteral("Unsupported operation type: %1").arg(operation.type);
            return result;
        }

        if (operationShape.IsNull())
        {
            result.errorMessage = operation.type == QStringLiteral("import_step")
                                      ? QStringLiteral("Imported STEP snapshot is not available for rebuild.")
                                      : QStringLiteral("Failed to build operation: %1").arg(operation.label);
            return result;
        }

        result.operationShapes.append({operation.id, operationShape});
        result.shape = operationShape;
    }

    result.success = !result.shape.IsNull();
    return result;
}

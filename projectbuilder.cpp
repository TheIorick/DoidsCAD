#include "projectbuilder.h"

#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <gp_Trsf.hxx>

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

TopoDS_Shape translatedShape(const TopoDS_Shape &shape, const OperationEntry &operation)
{
    const double x = parameterValue(operation, QStringLiteral("X"), 0.0);
    const double y = parameterValue(operation, QStringLiteral("Y"), 0.0);
    const double z = parameterValue(operation, QStringLiteral("Z"), 0.0);

    gp_Trsf transform;
    transform.SetTranslation(gp_Vec(x, y, z));
    return BRepBuilderAPI_Transform(shape, transform, Standard_True).Shape();
}

TopoDS_Shape findBuiltShape(const BuildResult &result, const int operationId)
{
    for (const OperationBuildShape &entry : result.operationShapes)
    {
        if (entry.operationId == operationId)
            return entry.shape;
    }

    return TopoDS_Shape();
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
            operationShape = translatedShape(BRepPrimAPI_MakeBox(length, width, height).Shape(), operation);
            result.description = operation.label;
        }
        else if (operation.type == QStringLiteral("cylinder"))
        {
            const double radius = parameterValue(operation, QStringLiteral("Radius"), 40.0);
            const double height = parameterValue(operation, QStringLiteral("Height"), 100.0);
            operationShape = translatedShape(BRepPrimAPI_MakeCylinder(radius, height).Shape(), operation);
            result.description = operation.label;
        }
        else if (operation.type == QStringLiteral("import_step"))
        {
            operationShape = importedShapeSnapshot;
            result.description = parameterText(operation, QStringLiteral("Source"));
        }
        else if (operation.type == QStringLiteral("fuse"))
        {
            const int leftId = static_cast<int>(parameterValue(operation, QStringLiteral("LeftId"), -1));
            const int rightId = static_cast<int>(parameterValue(operation, QStringLiteral("RightId"), -1));
            if (leftId < 0 || rightId < 0 || leftId == rightId)
            {
                result.errorMessage = QStringLiteral("Fuse requires two different operation ids.");
                return result;
            }

            const TopoDS_Shape leftShape = findBuiltShape(result, leftId);
            const TopoDS_Shape rightShape = findBuiltShape(result, rightId);
            if (leftShape.IsNull() || rightShape.IsNull())
            {
                result.errorMessage = QStringLiteral("Fuse references unknown operation results.");
                return result;
            }

            operationShape = BRepAlgoAPI_Fuse(leftShape, rightShape).Shape();
            result.description = operation.label;
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

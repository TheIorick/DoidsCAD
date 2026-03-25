#include "projectbuilder.h"

#include <BRepBndLib.hxx>
#include <BRep_Builder.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <Bnd_Box.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopExp_Explorer.hxx>
#include <gp_Pnt.hxx>
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

bool isBooleanOperation(const QString &type)
{
    return type == QStringLiteral("fuse") || type == QStringLiteral("cut");
}

bool isModifierOperation(const QString &type)
{
    return type == QStringLiteral("fillet");
}

TopoDS_Shape findBuiltShape(const BuildResult &result, int operationId);

QVector<int> referencedOperationIds(const ProjectModel &projectModel)
{
    QVector<int> ids;

    for (const OperationEntry &operation : projectModel.operations())
    {
        if (isBooleanOperation(operation.type))
        {
            const int leftId = static_cast<int>(parameterValue(operation, QStringLiteral("LeftId"), -1));
            const int rightId = static_cast<int>(parameterValue(operation, QStringLiteral("RightId"), -1));

            if (leftId >= 0 && !ids.contains(leftId))
                ids.append(leftId);

            if (rightId >= 0 && !ids.contains(rightId))
                ids.append(rightId);
        }
        else if (isModifierOperation(operation.type))
        {
            const int sourceId = static_cast<int>(parameterValue(operation, QStringLiteral("SourceId"), -1));

            if (sourceId >= 0 && !ids.contains(sourceId))
                ids.append(sourceId);
        }
    }

    return ids;
}

TopoDS_Shape composeSceneShape(const BuildResult &result, const ProjectModel &projectModel)
{
    const QVector<int> referencedIds = referencedOperationIds(projectModel);
    QVector<TopoDS_Shape> topLevelShapes;

    for (const OperationBuildShape &entry : result.operationShapes)
    {
        if (!referencedIds.contains(entry.operationId) && !entry.shape.IsNull())
            topLevelShapes.append(entry.shape);
    }

    if (topLevelShapes.isEmpty())
        return TopoDS_Shape();

    if (topLevelShapes.size() == 1)
        return topLevelShapes.constFirst();

    BRep_Builder builder;
    TopoDS_Compound compound;
    builder.MakeCompound(compound);

    for (const TopoDS_Shape &shape : topLevelShapes)
        builder.Add(compound, shape);

    return compound;
}

gp_Pnt boxPoint(const Bnd_Box &box, const QString &pointName)
{
    Standard_Real xmin = 0.0;
    Standard_Real ymin = 0.0;
    Standard_Real zmin = 0.0;
    Standard_Real xmax = 0.0;
    Standard_Real ymax = 0.0;
    Standard_Real zmax = 0.0;
    box.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    const double xCenter = (xmin + xmax) * 0.5;
    const double yCenter = (ymin + ymax) * 0.5;
    const double zCenter = (zmin + zmax) * 0.5;

    if (pointName == QStringLiteral("Left"))
        return gp_Pnt(xmin, yCenter, zCenter);
    if (pointName == QStringLiteral("Right"))
        return gp_Pnt(xmax, yCenter, zCenter);
    if (pointName == QStringLiteral("Front"))
        return gp_Pnt(xCenter, ymin, zCenter);
    if (pointName == QStringLiteral("Back"))
        return gp_Pnt(xCenter, ymax, zCenter);
    if (pointName == QStringLiteral("Bottom"))
        return gp_Pnt(xCenter, yCenter, zmin);
    if (pointName == QStringLiteral("Top"))
        return gp_Pnt(xCenter, yCenter, zmax);

    return gp_Pnt(xCenter, yCenter, zCenter);
}

gp_Pnt anchorPointForShape(const TopoDS_Shape &shape, const QString &pointName)
{
    Bnd_Box box;
    BRepBndLib::Add(shape, box);
    return boxPoint(box, pointName);
}

TopoDS_Shape placedPrimitiveShape(const TopoDS_Shape &shape,
                                  const OperationEntry &operation,
                                  const BuildResult &result,
                                  QString *errorMessage)
{
    const QString placementMode = parameterText(operation, QStringLiteral("PlacementMode")).toLower();
    const double x = parameterValue(operation, QStringLiteral("X"), 0.0);
    const double y = parameterValue(operation, QStringLiteral("Y"), 0.0);
    const double z = parameterValue(operation, QStringLiteral("Z"), 0.0);

    gp_Trsf transform;
    if (placementMode.isEmpty() || placementMode == QStringLiteral("absolute"))
    {
        transform.SetTranslation(gp_Vec(x, y, z));
        return BRepBuilderAPI_Transform(shape, transform, Standard_True).Shape();
    }

    if (placementMode != QStringLiteral("relative"))
    {
        if (errorMessage != nullptr)
            *errorMessage = QStringLiteral("Unsupported placement mode: %1").arg(placementMode);
        return TopoDS_Shape();
    }

    const int referenceId = static_cast<int>(parameterValue(operation, QStringLiteral("ReferenceId"), -1));
    const TopoDS_Shape referenceShape = findBuiltShape(result, referenceId);
    if (referenceShape.IsNull())
    {
        if (errorMessage != nullptr)
            *errorMessage = QStringLiteral("Relative placement references unknown operation result.");
        return TopoDS_Shape();
    }

    const gp_Pnt referencePoint = anchorPointForShape(
        referenceShape,
        parameterText(operation, QStringLiteral("ReferencePoint")));
    const gp_Pnt selfPoint = anchorPointForShape(
        shape,
        parameterText(operation, QStringLiteral("SelfPoint")));

    transform.SetTranslation(gp_Vec(referencePoint.X() - selfPoint.X() + x,
                                    referencePoint.Y() - selfPoint.Y() + y,
                                    referencePoint.Z() - selfPoint.Z() + z));
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
            operationShape = placedPrimitiveShape(BRepPrimAPI_MakeBox(length, width, height).Shape(),
                                                  operation,
                                                  result,
                                                  &result.errorMessage);
            result.description = operation.label;
        }
        else if (operation.type == QStringLiteral("cylinder"))
        {
            const double radius = parameterValue(operation, QStringLiteral("Radius"), 40.0);
            const double height = parameterValue(operation, QStringLiteral("Height"), 100.0);
            operationShape = placedPrimitiveShape(BRepPrimAPI_MakeCylinder(radius, height).Shape(),
                                                  operation,
                                                  result,
                                                  &result.errorMessage);
            result.description = operation.label;
        }
        else if (operation.type == QStringLiteral("cone"))
        {
            const double radius1 = parameterValue(operation, QStringLiteral("Radius1"), 40.0);
            const double radius2 = parameterValue(operation, QStringLiteral("Radius2"), 0.0);
            const double height = parameterValue(operation, QStringLiteral("Height"), 100.0);
            operationShape = placedPrimitiveShape(BRepPrimAPI_MakeCone(radius1, radius2, height).Shape(),
                                                  operation,
                                                  result,
                                                  &result.errorMessage);
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

            BRepAlgoAPI_Fuse fuseOp(leftShape, rightShape);
            fuseOp.Build();
            if (!fuseOp.IsDone())
            {
                result.errorMessage = QStringLiteral("Fuse operation failed: %1").arg(operation.label);
                return result;
            }
            operationShape = fuseOp.Shape();
            result.description = operation.label;
        }
        else if (operation.type == QStringLiteral("cut"))
        {
            const int leftId = static_cast<int>(parameterValue(operation, QStringLiteral("LeftId"), -1));
            const int rightId = static_cast<int>(parameterValue(operation, QStringLiteral("RightId"), -1));
            if (leftId < 0 || rightId < 0 || leftId == rightId)
            {
                result.errorMessage = QStringLiteral("Cut requires two different operation ids.");
                return result;
            }

            const TopoDS_Shape leftShape = findBuiltShape(result, leftId);
            const TopoDS_Shape rightShape = findBuiltShape(result, rightId);
            if (leftShape.IsNull() || rightShape.IsNull())
            {
                result.errorMessage = QStringLiteral("Cut references unknown operation results.");
                return result;
            }

            BRepAlgoAPI_Cut cutOp(leftShape, rightShape);
            cutOp.Build();
            if (!cutOp.IsDone())
            {
                result.errorMessage = QStringLiteral("Cut operation failed: %1").arg(operation.label);
                return result;
            }
            operationShape = cutOp.Shape();
            result.description = operation.label;
        }
        else if (operation.type == QStringLiteral("fillet"))
        {
            const int sourceId = static_cast<int>(parameterValue(operation, QStringLiteral("SourceId"), -1));
            if (sourceId < 0)
            {
                result.errorMessage = QStringLiteral("Fillet requires a source operation id.");
                return result;
            }

            const TopoDS_Shape sourceShape = findBuiltShape(result, sourceId);
            if (sourceShape.IsNull())
            {
                result.errorMessage = QStringLiteral("Fillet references unknown operation result.");
                return result;
            }

            const double radius = parameterValue(operation, QStringLiteral("Radius"), 5.0);
            BRepFilletAPI_MakeFillet filletOp(sourceShape);
            for (TopExp_Explorer exp(sourceShape, TopAbs_EDGE); exp.More(); exp.Next())
                filletOp.Add(radius, TopoDS::Edge(exp.Current()));

            filletOp.Build();
            if (!filletOp.IsDone())
            {
                result.errorMessage = QStringLiteral("Fillet operation failed: %1").arg(operation.label);
                return result;
            }
            operationShape = filletOp.Shape();
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
    }

    result.shape = composeSceneShape(result, projectModel);
    result.success = !result.shape.IsNull();
    return result;
}

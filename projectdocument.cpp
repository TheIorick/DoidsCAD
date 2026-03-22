#include "projectdocument.h"

#include "projectbuilder.h"

ProjectDocument::ProjectDocument()
{
    reset();
}

void ProjectDocument::reset()
{
    m_importedShapeSnapshot.Nullify();
    initializeStartupProject();
    rebuild();
}

void ProjectDocument::setShape(const TopoDS_Shape &shape, const QString &description)
{
    m_importedShapeSnapshot = shape;
    m_project.clear();
    m_project.addOperation(QStringLiteral("import_step"),
                           QStringLiteral("Import STEP"),
                           QStringLiteral("Done"),
                           {{QStringLiteral("Source"), description}});
    rebuild();
}

bool ProjectDocument::addBoxOperation(const double length, const double width, const double height)
{
    const int operationNumber = m_project.operationCount() + 1;
    m_project.addOperation(QStringLiteral("box"),
                           QStringLiteral("Box %1").arg(operationNumber),
                           QStringLiteral("Done"),
                           {{QStringLiteral("Length"), length},
                            {QStringLiteral("Width"), width},
                            {QStringLiteral("Height"), height}});
    return rebuild();
}

bool ProjectDocument::setOperationParameter(const int operationId, const QString &name, const QVariant &value)
{
    if (!m_project.setOperationParameter(operationId, name, value))
        return false;

    return rebuild();
}

bool ProjectDocument::rebuild()
{
    m_buildResult = ProjectBuilder::build(m_project, m_importedShapeSnapshot);
    return m_buildResult.success;
}

const TopoDS_Shape &ProjectDocument::shape() const
{
    return m_buildResult.shape;
}

bool ProjectDocument::hasShape() const
{
    return m_buildResult.success && !m_buildResult.shape.IsNull();
}

QString ProjectDocument::description() const
{
    return m_buildResult.description;
}

QString ProjectDocument::lastBuildError() const
{
    return m_buildResult.errorMessage;
}

TopoDS_Shape ProjectDocument::shapeForOperation(const int id) const
{
    for (const OperationBuildShape &entry : m_buildResult.operationShapes)
    {
        if (entry.operationId == id)
            return entry.shape;
    }

    return TopoDS_Shape();
}

const ProjectModel &ProjectDocument::project() const
{
    return m_project;
}

const OperationEntry *ProjectDocument::findOperation(const int id) const
{
    return m_project.findOperation(id);
}

void ProjectDocument::initializeStartupProject()
{
    m_project.clear();
    m_project.addOperation(QStringLiteral("box"),
                           QStringLiteral("Startup Box"),
                           QStringLiteral("Done"),
                           {{QStringLiteral("Length"), 120.0},
                            {QStringLiteral("Width"), 80.0},
                            {QStringLiteral("Height"), 60.0}});
}

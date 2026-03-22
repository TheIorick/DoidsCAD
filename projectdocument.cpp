#include "projectdocument.h"

#include <BRepPrimAPI_MakeBox.hxx>

ProjectDocument::ProjectDocument()
{
    reset();
}

void ProjectDocument::reset()
{
    m_shape = BRepPrimAPI_MakeBox(120.0, 80.0, 60.0).Shape();
    m_description = QStringLiteral("Startup box");
    initializeStartupProject();
}

void ProjectDocument::setShape(const TopoDS_Shape &shape, const QString &description)
{
    m_shape = shape;
    m_description = description;
    m_project.clear();
    m_project.addOperation(QStringLiteral("import_step"),
                           QStringLiteral("Import STEP"),
                           QStringLiteral("Done"),
                           {{QStringLiteral("Source"), description}});
}

const TopoDS_Shape &ProjectDocument::shape() const
{
    return m_shape;
}

bool ProjectDocument::hasShape() const
{
    return !m_shape.IsNull();
}

QString ProjectDocument::description() const
{
    return m_description;
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

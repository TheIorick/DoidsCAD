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
}

void ProjectDocument::setShape(const TopoDS_Shape &shape, const QString &description)
{
    m_shape = shape;
    m_description = description;
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

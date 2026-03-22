#ifndef PROJECTDOCUMENT_H
#define PROJECTDOCUMENT_H

#include "projectmodel.h"

#include <TopoDS_Shape.hxx>

#include <QString>

class ProjectDocument
{
public:
    ProjectDocument();

    void reset();
    void setShape(const TopoDS_Shape &shape, const QString &description = QString());

    const TopoDS_Shape &shape() const;
    bool hasShape() const;
    QString description() const;
    const ProjectModel &project() const;
    const OperationEntry *findOperation(int id) const;

private:
    void initializeStartupProject();

    ProjectModel m_project;
    TopoDS_Shape m_shape;
    QString m_description;
};

#endif // PROJECTDOCUMENT_H

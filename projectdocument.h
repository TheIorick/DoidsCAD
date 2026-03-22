#ifndef PROJECTDOCUMENT_H
#define PROJECTDOCUMENT_H

#include "buildresult.h"
#include "projectmodel.h"

#include <TopoDS_Shape.hxx>

#include <QString>

class ProjectDocument
{
public:
    ProjectDocument();

    void reset();
    void setShape(const TopoDS_Shape &shape, const QString &description = QString());
    bool addBoxOperation(double length, double width, double height);
    bool rebuild();

    const TopoDS_Shape &shape() const;
    bool hasShape() const;
    QString description() const;
    QString lastBuildError() const;
    TopoDS_Shape shapeForOperation(int id) const;
    const ProjectModel &project() const;
    const OperationEntry *findOperation(int id) const;

private:
    void initializeStartupProject();

    ProjectModel m_project;
    BuildResult m_buildResult;
    TopoDS_Shape m_importedShapeSnapshot;
};

#endif // PROJECTDOCUMENT_H

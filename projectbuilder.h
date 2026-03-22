#ifndef PROJECTBUILDER_H
#define PROJECTBUILDER_H

#include "buildresult.h"
#include "projectmodel.h"

class ProjectBuilder
{
public:
    static BuildResult build(const ProjectModel &projectModel,
                             const TopoDS_Shape &importedShapeSnapshot = TopoDS_Shape());
};

#endif // PROJECTBUILDER_H

#ifndef PROJECTDOCUMENT_H
#define PROJECTDOCUMENT_H

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

private:
    TopoDS_Shape m_shape;
    QString m_description;
};

#endif // PROJECTDOCUMENT_H

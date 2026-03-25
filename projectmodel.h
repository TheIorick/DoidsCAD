#ifndef PROJECTMODEL_H
#define PROJECTMODEL_H

#include "projecttypes.h"

#include <QVector>

class ProjectModel
{
public:
    ProjectModel();

    void clear();
    int addOperation(const QString &type,
                     const QString &label,
                     const QVector<OperationParameter> &parameters = {});

    const QVector<OperationEntry> &operations() const;
    OperationEntry *findOperation(int id);
    const OperationEntry *findOperation(int id) const;
    int operationCount() const;
    bool setOperationParameter(int operationId, const QString &name, const QVariant &value);

private:
    QVector<OperationEntry> m_operations;
    int m_nextOperationId;
};

#endif // PROJECTMODEL_H

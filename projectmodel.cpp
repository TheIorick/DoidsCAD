#include "projectmodel.h"

ProjectModel::ProjectModel()
    : m_nextOperationId(1)
{}

void ProjectModel::clear()
{
    m_operations.clear();
    m_nextOperationId = 1;
}

int ProjectModel::addOperation(const QString &type,
                               const QString &label,
                               const QString &state,
                               const QVector<OperationParameter> &parameters)
{
    OperationEntry entry;
    entry.id = m_nextOperationId++;
    entry.type = type;
    entry.label = label;
    entry.state = state;
    entry.parameters = parameters;
    m_operations.append(entry);
    return entry.id;
}

const QVector<OperationEntry> &ProjectModel::operations() const
{
    return m_operations;
}

const OperationEntry *ProjectModel::findOperation(const int id) const
{
    for (const OperationEntry &operation : m_operations)
    {
        if (operation.id == id)
            return &operation;
    }

    return nullptr;
}

int ProjectModel::operationCount() const
{
    return m_operations.size();
}

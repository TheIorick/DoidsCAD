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
                               const QVector<OperationParameter> &parameters)
{
    OperationEntry entry;
    entry.id = m_nextOperationId++;
    entry.type = type;
    entry.label = label;
    entry.parameters = parameters;
    m_operations.append(entry);
    return entry.id;
}

const QVector<OperationEntry> &ProjectModel::operations() const
{
    return m_operations;
}

OperationEntry *ProjectModel::findOperation(const int id)
{
    for (OperationEntry &operation : m_operations)
    {
        if (operation.id == id)
            return &operation;
    }

    return nullptr;
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

bool ProjectModel::setOperationParameter(const int operationId, const QString &name, const QVariant &value)
{
    OperationEntry *operation = findOperation(operationId);
    if (operation == nullptr)
        return false;

    for (OperationParameter &parameter : operation->parameters)
    {
        if (parameter.name == name)
        {
            parameter.value = value;
            return true;
        }
    }

    return false;
}

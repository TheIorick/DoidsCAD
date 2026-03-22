#ifndef PROJECTTYPES_H
#define PROJECTTYPES_H

#include <QString>
#include <QVector>
#include <QVariant>

struct OperationParameter
{
    QString name;
    QVariant value;
};

struct OperationEntry
{
    int id = 0;
    QString type;
    QString label;
    QString state;
    QVector<OperationParameter> parameters;
};

#endif // PROJECTTYPES_H

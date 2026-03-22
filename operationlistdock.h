#ifndef OPERATIONLISTDOCK_H
#define OPERATIONLISTDOCK_H

#include "projecttypes.h"

#include <QDockWidget>

class QTreeWidget;

class OperationListDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit OperationListDock(QWidget *parent = nullptr);
    void setOperations(const QVector<OperationEntry> &operations);
    void selectOperation(int operationId);

signals:
    void operationSelected(int operationId);

private:
    void handleCurrentItemChanged();

    QTreeWidget *m_treeWidget;
};

#endif // OPERATIONLISTDOCK_H

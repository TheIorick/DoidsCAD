#ifndef OPERATIONLISTDOCK_H
#define OPERATIONLISTDOCK_H

#include <QDockWidget>

class QTreeWidget;

class OperationListDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit OperationListDock(QWidget *parent = nullptr);

private:
    QTreeWidget *m_treeWidget;
};

#endif // OPERATIONLISTDOCK_H

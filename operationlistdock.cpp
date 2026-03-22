#include "operationlistdock.h"

#include <QHeaderView>
#include <QTreeWidget>

OperationListDock::OperationListDock(QWidget *parent)
    : QDockWidget(tr("Operation Tree"), parent)
    , m_treeWidget(new QTreeWidget(this))
{
    m_treeWidget->setColumnCount(2);
    m_treeWidget->setHeaderLabels({tr("Operation"), tr("State")});
    m_treeWidget->header()->setStretchLastSection(false);
    m_treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    setWidget(m_treeWidget);
}

void OperationListDock::setOperations(const QVector<OperationEntry> &operations)
{
    m_treeWidget->clear();

    auto *rootItem = new QTreeWidgetItem(m_treeWidget, {tr("Project"), tr("Ready")});
    for (const OperationEntry &operation : operations)
    {
        const QString title = tr("#%1 %2").arg(operation.id).arg(operation.label);
        auto *operationItem = new QTreeWidgetItem(rootItem, {title, operation.state});

        for (const OperationParameter &parameter : operation.parameters)
        {
            new QTreeWidgetItem(operationItem,
                                {tr("%1 = %2")
                                     .arg(parameter.name)
                                     .arg(parameter.value.toString()),
                                 tr("Parameter")});
        }
    }

    rootItem->setExpanded(true);
}

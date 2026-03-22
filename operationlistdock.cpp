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
    connect(m_treeWidget, &QTreeWidget::itemSelectionChanged, this, &OperationListDock::handleCurrentItemChanged);

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
        operationItem->setData(0, Qt::UserRole, operation.id);

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

void OperationListDock::handleCurrentItemChanged()
{
    const QTreeWidgetItem *item = m_treeWidget->currentItem();
    if (item == nullptr)
    {
        emit operationSelected(-1);
        return;
    }

    bool ok = false;
    const int operationId = item->data(0, Qt::UserRole).toInt(&ok);
    emit operationSelected(ok ? operationId : -1);
}

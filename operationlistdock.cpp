#include "operationlistdock.h"

#include <QHeaderView>
#include <QMap>
#include <QSignalBlocker>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>

namespace
{
// Returns the id of the logical parent operation, or -1 if root-level.
int parentOperationId(const OperationEntry &op)
{
    if (op.type == QLatin1String("box")
        || op.type == QLatin1String("cylinder")
        || op.type == QLatin1String("cone"))
    {
        QString placementMode;
        int referenceId = -1;
        for (const OperationParameter &p : op.parameters)
        {
            if (p.name == QLatin1String("PlacementMode"))
                placementMode = p.value.toString();
            else if (p.name == QLatin1String("ReferenceId"))
                referenceId = p.value.toInt();
        }
        if (placementMode == QLatin1String("relative") && referenceId > 0)
            return referenceId;
    }
    else if (op.type == QLatin1String("fillet"))
    {
        for (const OperationParameter &p : op.parameters)
        {
            if (p.name == QLatin1String("SourceId"))
                return p.value.toInt();
        }
    }
    return -1;
}
}

OperationListDock::OperationListDock(QWidget *parent)
    : QDockWidget(tr("Operation Tree"), parent)
    , m_treeWidget(new QTreeWidget(this))
{
    m_treeWidget->setColumnCount(2);
    m_treeWidget->setHeaderLabels({tr("Operation"), tr("Type")});
    m_treeWidget->header()->setStretchLastSection(false);
    m_treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    connect(m_treeWidget, &QTreeWidget::itemSelectionChanged, this, &OperationListDock::handleCurrentItemChanged);

    setWidget(m_treeWidget);
}

void OperationListDock::setOperations(const QVector<OperationEntry> &operations)
{
    const QSignalBlocker blocker(m_treeWidget);
    m_treeWidget->clear();

    auto *rootItem = new QTreeWidgetItem(m_treeWidget, {tr("Project"), QString()});

    // Build item map first
    QMap<int, QTreeWidgetItem *> itemMap;
    for (const OperationEntry &op : operations)
    {
        const QString title = QStringLiteral("#%1  %2").arg(op.id).arg(op.label);
        auto *item = new QTreeWidgetItem({title, op.type});
        item->setData(0, Qt::UserRole, op.id);
        itemMap[op.id] = item;
    }

    // Attach each item to its parent (or root)
    for (const OperationEntry &op : operations)
    {
        QTreeWidgetItem *item = itemMap.value(op.id);
        if (item == nullptr)
            continue;

        const int pid = parentOperationId(op);
        QTreeWidgetItem *parentItem = (pid > 0) ? itemMap.value(pid, rootItem) : rootItem;
        parentItem->addChild(item);
    }

    // Expand everything
    m_treeWidget->expandAll();
}

void OperationListDock::selectOperation(const int operationId)
{
    const QSignalBlocker blocker(m_treeWidget);

    if (operationId < 0)
    {
        m_treeWidget->clearSelection();
        m_treeWidget->setCurrentItem(nullptr);
        return;
    }

    // Search all items recursively via QTreeWidgetItemIterator
    QTreeWidgetItemIterator it(m_treeWidget);
    while (*it)
    {
        bool ok = false;
        const int itemId = (*it)->data(0, Qt::UserRole).toInt(&ok);
        if (ok && itemId == operationId)
        {
            m_treeWidget->setCurrentItem(*it);
            (*it)->setSelected(true);
            return;
        }
        ++it;
    }
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

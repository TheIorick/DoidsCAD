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

    auto *rootItem = new QTreeWidgetItem(m_treeWidget, {tr("Project"), tr("Idle")});
    new QTreeWidgetItem(rootItem, {tr("Import STEP"), tr("Planned")});
    new QTreeWidgetItem(rootItem, {tr("Build Cylinder"), tr("Planned")});
    new QTreeWidgetItem(rootItem, {tr("Chamfer"), tr("Planned")});
    rootItem->setExpanded(true);

    setWidget(m_treeWidget);
}

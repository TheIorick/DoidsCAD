#include "propertyeditordock.h"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>

PropertyEditorDock::PropertyEditorDock(QWidget *parent)
    : QDockWidget(tr("Properties"), parent)
    , m_tableWidget(new QTableWidget(4, 2, this))
{
    m_tableWidget->setHorizontalHeaderLabels({tr("Parameter"), tr("Value")});
    m_tableWidget->verticalHeader()->setVisible(false);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_tableWidget->setItem(0, 0, new QTableWidgetItem(tr("Selection")));
    m_tableWidget->setItem(0, 1, new QTableWidgetItem(tr("None")));
    m_tableWidget->setItem(1, 0, new QTableWidgetItem(tr("Format")));
    m_tableWidget->setItem(1, 1, new QTableWidgetItem(tr("STEP")));
    m_tableWidget->setItem(2, 0, new QTableWidgetItem(tr("Compiler")));
    m_tableWidget->setItem(2, 1, new QTableWidgetItem(tr("MSVC")));
    m_tableWidget->setItem(3, 0, new QTableWidgetItem(tr("Viewer")));
    m_tableWidget->setItem(3, 1, new QTableWidgetItem(tr("OpenCascade - pending integration")));

    setWidget(m_tableWidget);
}

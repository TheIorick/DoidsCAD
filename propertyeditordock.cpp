#include "propertyeditordock.h"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>

PropertyEditorDock::PropertyEditorDock(QWidget *parent)
    : QDockWidget(tr("Properties"), parent)
    , m_tableWidget(new QTableWidget(4, 2, this))
    , m_currentOperationId(-1)
    , m_isUpdating(false)
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
    m_tableWidget->setItem(3, 1, new QTableWidgetItem(tr("OpenCascade")));
    connect(m_tableWidget, &QTableWidget::cellChanged, this, &PropertyEditorDock::handleCellChanged);

    setWidget(m_tableWidget);
}

void PropertyEditorDock::setSelectionDescription(const QString &description)
{
    QTableWidgetItem *selectionValueItem = m_tableWidget->item(0, 1);
    if (selectionValueItem == nullptr)
    {
        selectionValueItem = new QTableWidgetItem();
        m_tableWidget->setItem(0, 1, selectionValueItem);
    }

    selectionValueItem->setText(description);
}

void PropertyEditorDock::showOperationDetails(const OperationEntry *operation)
{
    m_isUpdating = true;
    m_tableWidget->clearContents();

    if (operation == nullptr)
    {
        m_currentOperationId = -1;
        m_tableWidget->setRowCount(4);
        m_tableWidget->setItem(0, 0, new QTableWidgetItem(tr("Selection")));
        m_tableWidget->setItem(0, 1, new QTableWidgetItem(tr("None")));
        m_tableWidget->setItem(1, 0, new QTableWidgetItem(tr("Format")));
        m_tableWidget->setItem(1, 1, new QTableWidgetItem(tr("STEP")));
        m_tableWidget->setItem(2, 0, new QTableWidgetItem(tr("Compiler")));
        m_tableWidget->setItem(2, 1, new QTableWidgetItem(tr("MSVC")));
        m_tableWidget->setItem(3, 0, new QTableWidgetItem(tr("Viewer")));
        m_tableWidget->setItem(3, 1, new QTableWidgetItem(tr("OpenCascade")));
        m_isUpdating = false;
        return;
    }

    m_currentOperationId = operation->id;
    const int rowCount = 3 + operation->parameters.size();
    m_tableWidget->setRowCount(rowCount);
    m_tableWidget->setItem(0, 0, new QTableWidgetItem(tr("Operation")));
    m_tableWidget->setItem(0, 1, new QTableWidgetItem(operation->label));
    m_tableWidget->setItem(1, 0, new QTableWidgetItem(tr("Type")));
    m_tableWidget->setItem(1, 1, new QTableWidgetItem(operation->type));
    m_tableWidget->setItem(2, 0, new QTableWidgetItem(tr("State")));
    m_tableWidget->setItem(2, 1, new QTableWidgetItem(operation->state));

    for (int i = 0; i < operation->parameters.size(); ++i)
    {
        const OperationParameter &parameter = operation->parameters.at(i);
        m_tableWidget->setItem(3 + i, 0, new QTableWidgetItem(parameter.name));
        auto *valueItem = new QTableWidgetItem(parameter.value.toString());
        if (operation->type != QLatin1String("box"))
            valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable);

        m_tableWidget->setItem(3 + i, 1, valueItem);
    }

    m_isUpdating = false;
}

void PropertyEditorDock::handleCellChanged(const int row, const int column)
{
    if (m_isUpdating || m_currentOperationId < 0 || column != 1 || row < 3)
        return;

    const QTableWidgetItem *nameItem = m_tableWidget->item(row, 0);
    const QTableWidgetItem *valueItem = m_tableWidget->item(row, 1);
    if (nameItem == nullptr || valueItem == nullptr)
        return;

    bool ok = false;
    const double numericValue = valueItem->text().toDouble(&ok);
    if (!ok)
        return;

    emit operationParameterEdited(m_currentOperationId, nameItem->text(), numericValue);
}

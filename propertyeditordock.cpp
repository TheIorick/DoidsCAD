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
    m_tableWidget->setEditTriggers(QAbstractItemView::DoubleClicked
                                   | QAbstractItemView::EditKeyPressed
                                   | QAbstractItemView::SelectedClicked);

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
        auto *label0 = new QTableWidgetItem(tr("Selection"));
        auto *value0 = new QTableWidgetItem(tr("None"));
        auto *label1 = new QTableWidgetItem(tr("Format"));
        auto *value1 = new QTableWidgetItem(tr("STEP"));
        auto *label2 = new QTableWidgetItem(tr("Compiler"));
        auto *value2 = new QTableWidgetItem(tr("MSVC"));
        auto *label3 = new QTableWidgetItem(tr("Viewer"));
        auto *value3 = new QTableWidgetItem(tr("OpenCascade"));

        label0->setFlags(label0->flags() & ~Qt::ItemIsEditable);
        value0->setFlags(value0->flags() & ~Qt::ItemIsEditable);
        label1->setFlags(label1->flags() & ~Qt::ItemIsEditable);
        value1->setFlags(value1->flags() & ~Qt::ItemIsEditable);
        label2->setFlags(label2->flags() & ~Qt::ItemIsEditable);
        value2->setFlags(value2->flags() & ~Qt::ItemIsEditable);
        label3->setFlags(label3->flags() & ~Qt::ItemIsEditable);
        value3->setFlags(value3->flags() & ~Qt::ItemIsEditable);

        m_tableWidget->setItem(0, 0, label0);
        m_tableWidget->setItem(0, 1, value0);
        m_tableWidget->setItem(1, 0, label1);
        m_tableWidget->setItem(1, 1, value1);
        m_tableWidget->setItem(2, 0, label2);
        m_tableWidget->setItem(2, 1, value2);
        m_tableWidget->setItem(3, 0, label3);
        m_tableWidget->setItem(3, 1, value3);
        m_isUpdating = false;
        return;
    }

    m_currentOperationId = operation->id;
    const int rowCount = 3 + operation->parameters.size();
    m_tableWidget->setRowCount(rowCount);
    auto *operationLabelItem = new QTableWidgetItem(tr("Operation"));
    auto *operationValueItem = new QTableWidgetItem(operation->label);
    auto *typeLabelItem = new QTableWidgetItem(tr("Type"));
    auto *typeValueItem = new QTableWidgetItem(operation->type);
    auto *stateLabelItem = new QTableWidgetItem(tr("State"));
    auto *stateValueItem = new QTableWidgetItem(operation->state);

    operationLabelItem->setFlags(operationLabelItem->flags() & ~Qt::ItemIsEditable);
    operationValueItem->setFlags(operationValueItem->flags() & ~Qt::ItemIsEditable);
    typeLabelItem->setFlags(typeLabelItem->flags() & ~Qt::ItemIsEditable);
    typeValueItem->setFlags(typeValueItem->flags() & ~Qt::ItemIsEditable);
    stateLabelItem->setFlags(stateLabelItem->flags() & ~Qt::ItemIsEditable);
    stateValueItem->setFlags(stateValueItem->flags() & ~Qt::ItemIsEditable);

    m_tableWidget->setItem(0, 0, operationLabelItem);
    m_tableWidget->setItem(0, 1, operationValueItem);
    m_tableWidget->setItem(1, 0, typeLabelItem);
    m_tableWidget->setItem(1, 1, typeValueItem);
    m_tableWidget->setItem(2, 0, stateLabelItem);
    m_tableWidget->setItem(2, 1, stateValueItem);

    for (int i = 0; i < operation->parameters.size(); ++i)
    {
        const OperationParameter &parameter = operation->parameters.at(i);
        auto *nameItem = new QTableWidgetItem(parameter.name);
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        m_tableWidget->setItem(3 + i, 0, nameItem);
        auto *valueItem = new QTableWidgetItem(parameter.value.toString());
        if (operation->type != QLatin1String("box")
            && operation->type != QLatin1String("cylinder"))
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

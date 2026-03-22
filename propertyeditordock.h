#ifndef PROPERTYEDITORDOCK_H
#define PROPERTYEDITORDOCK_H

#include "projecttypes.h"

#include <QDockWidget>

class QComboBox;
class QTableWidget;

class PropertyEditorDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit PropertyEditorDock(QWidget *parent = nullptr);
    void setSelectionDescription(const QString &description);
    void showOperationDetails(const OperationEntry *operation, const QVector<OperationEntry> &allOperations);

signals:
    void operationParameterEdited(int operationId, const QString &name, const QVariant &value);

private:
    void handleCellChanged(int row, int column);
    void handleOperationReferenceChanged(int operationId, const QString &name, QComboBox *comboBox);

    QTableWidget *m_tableWidget;
    int m_currentOperationId;
    bool m_isUpdating;
};

#endif // PROPERTYEDITORDOCK_H

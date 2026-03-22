#ifndef PROPERTYEDITORDOCK_H
#define PROPERTYEDITORDOCK_H

#include "projecttypes.h"

#include <QDockWidget>

class QTableWidget;

class PropertyEditorDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit PropertyEditorDock(QWidget *parent = nullptr);
    void setSelectionDescription(const QString &description);
    void showOperationDetails(const OperationEntry *operation);

private:
    QTableWidget *m_tableWidget;
};

#endif // PROPERTYEDITORDOCK_H

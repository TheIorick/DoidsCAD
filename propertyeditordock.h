#ifndef PROPERTYEDITORDOCK_H
#define PROPERTYEDITORDOCK_H

#include "projecttypes.h"

#include <QDockWidget>

class QLabel;
class QVBoxLayout;
class QWidget;

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
    QWidget *createPlaceholderWidget() const;
    QWidget *createOperationWidget(const OperationEntry *operation, const QVector<OperationEntry> &allOperations);
    void replaceDetailsWidget(QWidget *widget);

    QWidget *m_containerWidget;
    QVBoxLayout *m_rootLayout;
    QLabel *m_selectionValueLabel;
    QWidget *m_detailsWidget;
    bool m_isUpdating;
};

#endif // PROPERTYEDITORDOCK_H

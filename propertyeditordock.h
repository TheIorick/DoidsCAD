#ifndef PROPERTYEDITORDOCK_H
#define PROPERTYEDITORDOCK_H

#include <QDockWidget>

class QTableWidget;

class PropertyEditorDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit PropertyEditorDock(QWidget *parent = nullptr);

private:
    QTableWidget *m_tableWidget;
};

#endif // PROPERTYEDITORDOCK_H

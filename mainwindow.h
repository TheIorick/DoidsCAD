#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QAction;
class CadViewport;
class OperationListDock;
class PropertyEditorDock;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showNotImplementedMessage();

private:
    void createActions();
    void createMenus();
    void createToolBar();
    void createDocks();
    void configureWindow();

    CadViewport *m_viewport;
    OperationListDock *m_operationDock;
    PropertyEditorDock *m_propertyDock;

    QAction *m_newProjectAction;
    QAction *m_importStepAction;
    QAction *m_exportStepAction;
    QAction *m_exitAction;
    QAction *m_fitViewAction;
    QAction *m_wireframeAction;
    QAction *m_shadedAction;
};

#endif // MAINWINDOW_H

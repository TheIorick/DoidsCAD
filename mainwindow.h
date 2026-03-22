#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QAction;
class CadViewport;
class OperationListDock;
class ProjectDocument;
class PropertyEditorDock;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void newProject();
    void showNotImplementedMessage();
    void importStep();
    void exportStep();
    void updateSelectionDescription(const QString &description);

private:
    void createActions();
    void createMenus();
    void createToolBar();
    void createDocks();
    void configureWindow();
    void refreshViewport();
    void refreshOperationTree();

    CadViewport *m_viewport;
    OperationListDock *m_operationDock;
    ProjectDocument *m_projectDocument;
    PropertyEditorDock *m_propertyDock;

    QAction *m_newProjectAction;
    QAction *m_importStepAction;
    QAction *m_exportStepAction;
    QAction *m_exitAction;
    QAction *m_fitViewAction;
    QAction *m_wireframeAction;
    QAction *m_shadedAction;
    QAction *m_frontViewAction;
    QAction *m_topViewAction;
    QAction *m_rightViewAction;
    QAction *m_isometricViewAction;
};

#endif // MAINWINDOW_H

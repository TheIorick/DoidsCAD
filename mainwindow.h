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
    void addBoxOperation();
    void showNotImplementedMessage();
    void importStep();
    void exportStep();
    void updateOperationParameter(int operationId, const QString &name, const QVariant &value);
    void showOperationDetails(int operationId);
    void updateSelectionDescription(const QString &description);

private:
    void createActions();
    void createMenus();
    void createToolBar();
    void createDocks();
    void configureWindow();
    void refreshViewport();
    void refreshOperationTree();
    void selectOperation(int operationId);

    CadViewport *m_viewport;
    OperationListDock *m_operationDock;
    ProjectDocument *m_projectDocument;
    PropertyEditorDock *m_propertyDock;
    int m_selectedOperationId;

    QAction *m_newProjectAction;
    QAction *m_importStepAction;
    QAction *m_exportStepAction;
    QAction *m_exitAction;
    QAction *m_addBoxAction;
    QAction *m_fitViewAction;
    QAction *m_wireframeAction;
    QAction *m_shadedAction;
    QAction *m_frontViewAction;
    QAction *m_topViewAction;
    QAction *m_rightViewAction;
    QAction *m_isometricViewAction;
};

#endif // MAINWINDOW_H

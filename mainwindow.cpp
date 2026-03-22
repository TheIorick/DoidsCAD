#include "mainwindow.h"

#include "cadviewport.h"
#include "operationlistdock.h"
#include "projectdocument.h"
#include "propertyeditordock.h"
#include "stepexchange.h"

#include <QAction>
#include <QFileDialog>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_viewport(new CadViewport(this))
    , m_operationDock(new OperationListDock(this))
    , m_projectDocument(new ProjectDocument())
    , m_propertyDock(new PropertyEditorDock(this))
    , m_selectedOperationId(-1)
    , m_newProjectAction(nullptr)
    , m_importStepAction(nullptr)
    , m_exportStepAction(nullptr)
    , m_exitAction(nullptr)
    , m_addBoxAction(nullptr)
    , m_addCylinderAction(nullptr)
    , m_addFuseAction(nullptr)
    , m_fitViewAction(nullptr)
    , m_wireframeAction(nullptr)
    , m_shadedAction(nullptr)
    , m_frontViewAction(nullptr)
    , m_topViewAction(nullptr)
    , m_rightViewAction(nullptr)
    , m_isometricViewAction(nullptr)
{
    configureWindow();
    createActions();
    createMenus();
    createToolBar();
    createDocks();

    setCentralWidget(m_viewport);
    refreshViewport();
    statusBar()->showMessage(tr("OpenCascade viewer is connected. Current document: %1").arg(m_projectDocument->description()));
}

MainWindow::~MainWindow()
{
    delete m_projectDocument;
}

void MainWindow::newProject()
{
    m_projectDocument->reset();
    refreshViewport();
    statusBar()->showMessage(tr("Started a new project with the default startup solid."), 5000);
}

void MainWindow::addBoxOperation()
{
    const int index = m_projectDocument->project().operationCount();
    const double length = 120.0 + index * 20.0;
    const double width = 80.0 + index * 10.0;
    const double height = 60.0 + index * 5.0;

    if (!m_projectDocument->addBoxOperation(length, width, height))
    {
        QMessageBox::critical(this, tr("Rebuild Failed"), m_projectDocument->lastBuildError());
        return;
    }

    refreshViewport();
    statusBar()->showMessage(tr("Added Box operation: %1 x %2 x %3").arg(length).arg(width).arg(height), 5000);
}

void MainWindow::addCylinderOperation()
{
    const int index = m_projectDocument->project().operationCount();
    const double radius = 40.0 + index * 5.0;
    const double height = 100.0 + index * 15.0;

    if (!m_projectDocument->addCylinderOperation(radius, height))
    {
        QMessageBox::critical(this, tr("Rebuild Failed"), m_projectDocument->lastBuildError());
        return;
    }

    refreshViewport();
    statusBar()->showMessage(tr("Added Cylinder operation: R%1 H%2").arg(radius).arg(height), 5000);
}

void MainWindow::addFuseOperation()
{
    if (!m_projectDocument->addFuseOperation())
    {
        QMessageBox::critical(this, tr("Rebuild Failed"), m_projectDocument->lastBuildError());
        return;
    }

    refreshViewport();
    statusBar()->showMessage(tr("Added Fuse operation for the last two results."), 5000);
}

void MainWindow::showNotImplementedMessage()
{
    const auto *action = qobject_cast<QAction *>(sender());
    const QString actionName = action ? action->text() : tr("Action");

    QMessageBox::information(
        this,
        tr("Planned Action"),
        tr("%1 is part of the current implementation plan and will be connected in the next steps.")
            .arg(actionName));
}

void MainWindow::importStep()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Import STEP"),
        QString(),
        tr("STEP Files (*.step *.stp)"));

    if (filePath.isEmpty())
        return;

    QString errorMessage;
    TopoDS_Shape importedShape;
    if (!StepExchange::importStep(filePath, importedShape, &errorMessage))
    {
        QMessageBox::critical(this, tr("STEP Import Failed"), errorMessage);
        return;
    }

    m_projectDocument->setShape(importedShape, tr("Imported STEP"));
    refreshViewport();
    statusBar()->showMessage(tr("Imported STEP: %1").arg(filePath), 5000);
}

void MainWindow::exportStep()
{
    if (!m_projectDocument->hasShape())
    {
        QMessageBox::information(this, tr("Export STEP"), tr("There is no model to export."));
        return;
    }

    const QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("Export STEP"),
        QString(),
        tr("STEP Files (*.step *.stp)"));

    if (filePath.isEmpty())
        return;

    QString normalizedPath = filePath;
    if (!normalizedPath.endsWith(".step", Qt::CaseInsensitive)
        && !normalizedPath.endsWith(".stp", Qt::CaseInsensitive))
    {
        normalizedPath += ".step";
    }

    QString errorMessage;
    if (!StepExchange::exportStep(normalizedPath, m_projectDocument->shape(), &errorMessage))
    {
        QMessageBox::critical(this, tr("STEP Export Failed"), errorMessage);
        return;
    }

    statusBar()->showMessage(tr("Exported STEP: %1").arg(normalizedPath), 5000);
}

void MainWindow::updateSelectionDescription(const QString &description)
{
    m_propertyDock->setSelectionDescription(description);
    statusBar()->showMessage(tr("Selection: %1").arg(description), 3000);
}

void MainWindow::showOperationDetails(const int operationId)
{
    m_selectedOperationId = operationId;
    refreshDisplayedShape();
    m_propertyDock->showOperationDetails(m_projectDocument->findOperation(operationId),
                                         m_projectDocument->project().operations());

    if (operationId >= 0)
        statusBar()->showMessage(tr("Operation #%1 selected").arg(operationId), 3000);
}

void MainWindow::updateOperationParameter(const int operationId, const QString &name, const QVariant &value)
{
    if (!m_projectDocument->setOperationParameter(operationId, name, value))
    {
        QMessageBox::critical(this, tr("Rebuild Failed"), m_projectDocument->lastBuildError());
        return;
    }

    refreshViewport();
    selectOperation(operationId);
    statusBar()->showMessage(tr("Updated %1 for operation #%2").arg(name).arg(operationId), 5000);
}

void MainWindow::refreshViewport()
{
    refreshDisplayedShape();
    refreshOperationTree();
}

void MainWindow::refreshOperationTree()
{
    m_operationDock->setOperations(m_projectDocument->project().operations());
    m_operationDock->selectOperation(m_selectedOperationId);
}

void MainWindow::selectOperation(const int operationId)
{
    m_selectedOperationId = operationId;
    m_operationDock->selectOperation(operationId);
    showOperationDetails(operationId);
}

void MainWindow::refreshDisplayedShape()
{
    if (m_selectedOperationId >= 0)
    {
        const TopoDS_Shape operationShape = m_projectDocument->shapeForOperation(m_selectedOperationId);
        if (!operationShape.IsNull())
        {
            m_viewport->setShape(operationShape);
            m_viewport->setDisplayedShapeSelected(true);
            return;
        }
    }

    if (m_projectDocument->hasShape())
        m_viewport->setShape(m_projectDocument->shape());
    else
        m_viewport->clearShape();
}

void MainWindow::createActions()
{
    m_newProjectAction = new QAction(tr("New Project"), this);
    m_newProjectAction->setShortcut(QKeySequence::New);

    m_importStepAction = new QAction(tr("Import STEP..."), this);
    m_importStepAction->setShortcut(tr("Ctrl+I"));

    m_exportStepAction = new QAction(tr("Export STEP..."), this);
    m_exportStepAction->setShortcut(tr("Ctrl+E"));

    m_exitAction = new QAction(tr("Exit"), this);
    m_exitAction->setShortcut(QKeySequence::Quit);

    m_addBoxAction = new QAction(tr("Add Box"), this);
    m_addCylinderAction = new QAction(tr("Add Cylinder"), this);
    m_addFuseAction = new QAction(tr("Add Fuse"), this);

    m_fitViewAction = new QAction(tr("Fit View"), this);
    m_wireframeAction = new QAction(tr("Wireframe"), this);
    m_shadedAction = new QAction(tr("Shaded"), this);
    m_frontViewAction = new QAction(tr("Front"), this);
    m_topViewAction = new QAction(tr("Top"), this);
    m_rightViewAction = new QAction(tr("Right"), this);
    m_isometricViewAction = new QAction(tr("Isometric"), this);

    connect(m_newProjectAction, &QAction::triggered, this, &MainWindow::newProject);
    connect(m_addBoxAction, &QAction::triggered, this, &MainWindow::addBoxOperation);
    connect(m_addCylinderAction, &QAction::triggered, this, &MainWindow::addCylinderOperation);
    connect(m_addFuseAction, &QAction::triggered, this, &MainWindow::addFuseOperation);
    connect(m_importStepAction, &QAction::triggered, this, &MainWindow::importStep);
    connect(m_exportStepAction, &QAction::triggered, this, &MainWindow::exportStep);
    connect(m_fitViewAction, &QAction::triggered, m_viewport, &CadViewport::fitAll);
    connect(m_wireframeAction, &QAction::triggered, m_viewport, &CadViewport::setWireframeMode);
    connect(m_shadedAction, &QAction::triggered, m_viewport, &CadViewport::setShadedMode);
    connect(m_frontViewAction, &QAction::triggered, m_viewport, &CadViewport::setFrontView);
    connect(m_topViewAction, &QAction::triggered, m_viewport, &CadViewport::setTopView);
    connect(m_rightViewAction, &QAction::triggered, m_viewport, &CadViewport::setRightView);
    connect(m_isometricViewAction, &QAction::triggered, m_viewport, &CadViewport::setIsometricView);
    connect(m_viewport, &CadViewport::selectionDescriptionChanged, this, &MainWindow::updateSelectionDescription);
    connect(m_operationDock, &OperationListDock::operationSelected, this, &MainWindow::showOperationDetails);
    connect(m_propertyDock, &PropertyEditorDock::operationParameterEdited, this, &MainWindow::updateOperationParameter);
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);
}

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addAction(m_newProjectAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_importStepAction);
    fileMenu->addAction(m_exportStepAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAction);

    QMenu *viewMenu = menuBar()->addMenu(tr("View"));
    viewMenu->addAction(m_fitViewAction);
    viewMenu->addSeparator();
    viewMenu->addAction(m_frontViewAction);
    viewMenu->addAction(m_topViewAction);
    viewMenu->addAction(m_rightViewAction);
    viewMenu->addAction(m_isometricViewAction);
    viewMenu->addSeparator();
    viewMenu->addAction(m_wireframeAction);
    viewMenu->addAction(m_shadedAction);

    QMenu *modelMenu = menuBar()->addMenu(tr("Model"));
    modelMenu->addAction(m_addBoxAction);
    modelMenu->addAction(m_addCylinderAction);
    modelMenu->addAction(m_addFuseAction);
    menuBar()->addMenu(tr("Help"));
}

void MainWindow::createToolBar()
{
    QToolBar *mainToolBar = addToolBar(tr("Main Toolbar"));
    mainToolBar->setMovable(false);
    mainToolBar->addAction(m_newProjectAction);
    mainToolBar->addAction(m_addBoxAction);
    mainToolBar->addAction(m_addCylinderAction);
    mainToolBar->addAction(m_addFuseAction);
    mainToolBar->addAction(m_importStepAction);
    mainToolBar->addAction(m_exportStepAction);
    mainToolBar->addSeparator();
    mainToolBar->addAction(m_fitViewAction);
    mainToolBar->addAction(m_frontViewAction);
    mainToolBar->addAction(m_topViewAction);
    mainToolBar->addAction(m_rightViewAction);
    mainToolBar->addAction(m_isometricViewAction);
    mainToolBar->addSeparator();
    mainToolBar->addAction(m_wireframeAction);
    mainToolBar->addAction(m_shadedAction);
}

void MainWindow::createDocks()
{
    addDockWidget(Qt::LeftDockWidgetArea, m_operationDock);
    addDockWidget(Qt::RightDockWidgetArea, m_propertyDock);
}

void MainWindow::configureWindow()
{
    resize(1280, 800);
    setWindowTitle(tr("DOIDS - Mini CAD"));
    setDockOptions(QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks);
}

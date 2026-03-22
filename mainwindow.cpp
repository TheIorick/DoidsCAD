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
    , m_newProjectAction(nullptr)
    , m_importStepAction(nullptr)
    , m_exportStepAction(nullptr)
    , m_exitAction(nullptr)
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

void MainWindow::refreshViewport()
{
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

    m_fitViewAction = new QAction(tr("Fit View"), this);
    m_wireframeAction = new QAction(tr("Wireframe"), this);
    m_shadedAction = new QAction(tr("Shaded"), this);
    m_frontViewAction = new QAction(tr("Front"), this);
    m_topViewAction = new QAction(tr("Top"), this);
    m_rightViewAction = new QAction(tr("Right"), this);
    m_isometricViewAction = new QAction(tr("Isometric"), this);

    connect(m_newProjectAction, &QAction::triggered, this, &MainWindow::newProject);
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

    menuBar()->addMenu(tr("Model"));
    menuBar()->addMenu(tr("Help"));
}

void MainWindow::createToolBar()
{
    QToolBar *mainToolBar = addToolBar(tr("Main Toolbar"));
    mainToolBar->setMovable(false);
    mainToolBar->addAction(m_newProjectAction);
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

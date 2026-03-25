#include "mainwindow.h"

#include "cadviewport.h"
#include "operationlistdock.h"
#include "projectdocument.h"
#include "propertyeditordock.h"
#include "stepexchange.h"

#include <QAction>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>

namespace
{
struct BooleanOperationParameters
{
    int leftId = -1;
    int rightId = -1;
};

bool promptBooleanParameters(QWidget *parent,
                             const QString &title,
                             const QVector<OperationEntry> &sources,
                             BooleanOperationParameters *parameters)
{
    if (parameters == nullptr || sources.size() < 2)
        return false;

    QDialog dialog(parent);
    dialog.setWindowTitle(title);

    auto *layout = new QFormLayout(&dialog);
    auto *leftComboBox = new QComboBox(&dialog);
    auto *rightComboBox = new QComboBox(&dialog);

    for (const OperationEntry &operation : sources)
    {
        const QString label = QStringLiteral("#%1 %2").arg(operation.id).arg(operation.label);
        leftComboBox->addItem(label, operation.id);
        rightComboBox->addItem(label, operation.id);
    }

    const int leftIndex = leftComboBox->findData(parameters->leftId);
    const int rightIndex = rightComboBox->findData(parameters->rightId);
    leftComboBox->setCurrentIndex(leftIndex >= 0 ? leftIndex : 0);
    rightComboBox->setCurrentIndex(rightIndex >= 0 ? rightIndex : (rightComboBox->count() > 1 ? 1 : 0));

    layout->addRow(QObject::tr("Left"), leftComboBox);
    layout->addRow(QObject::tr("Right"), rightComboBox);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addRow(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
        return false;

    parameters->leftId = leftComboBox->currentData().toInt();
    parameters->rightId = rightComboBox->currentData().toInt();
    return true;
}
}

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
    , m_addConeAction(nullptr)
    , m_addFilletAction(nullptr)
    , m_addFuseAction(nullptr)
    , m_addCutAction(nullptr)
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
    statusBar()->showMessage(tr("Ready. Select an operation and add a new one to attach it automatically."));
}

MainWindow::~MainWindow()
{
    delete m_projectDocument;
}

void MainWindow::newProject()
{
    m_selectedOperationId = -1;
    m_projectDocument->reset();
    refreshViewport();
    statusBar()->showMessage(tr("New project started."), 4000);
}

void MainWindow::addBoxOperation()
{
    const OperationEntry *reference = m_projectDocument->findOperation(m_selectedOperationId);
    const QString mode = reference ? QStringLiteral("relative") : QStringLiteral("absolute");
    const int refId = reference ? reference->id : -1;

    if (!m_projectDocument->addBoxOperation(120.0, 80.0, 60.0,
                                            0.0, 0.0, 0.0,
                                            mode, refId,
                                            QStringLiteral("Right"),
                                            QStringLiteral("Left")))
    {
        QMessageBox::critical(this, tr("Rebuild Failed"), m_projectDocument->lastBuildError());
        return;
    }

    const int newId = m_projectDocument->lastOperationId();
    refreshViewport();
    selectOperation(newId);
    statusBar()->showMessage(tr("Added Box%1").arg(
        reference ? tr(" relative to #%1").arg(refId) : QString()), 4000);
}

void MainWindow::addCylinderOperation()
{
    const OperationEntry *reference = m_projectDocument->findOperation(m_selectedOperationId);
    const QString mode = reference ? QStringLiteral("relative") : QStringLiteral("absolute");
    const int refId = reference ? reference->id : -1;

    if (!m_projectDocument->addCylinderOperation(40.0, 100.0,
                                                  0.0, 0.0, 0.0,
                                                  mode, refId,
                                                  QStringLiteral("Right"),
                                                  QStringLiteral("Left")))
    {
        QMessageBox::critical(this, tr("Rebuild Failed"), m_projectDocument->lastBuildError());
        return;
    }

    const int newId = m_projectDocument->lastOperationId();
    refreshViewport();
    selectOperation(newId);
    statusBar()->showMessage(tr("Added Cylinder%1").arg(
        reference ? tr(" relative to #%1").arg(refId) : QString()), 4000);
}

void MainWindow::addConeOperation()
{
    const OperationEntry *reference = m_projectDocument->findOperation(m_selectedOperationId);
    const QString mode = reference ? QStringLiteral("relative") : QStringLiteral("absolute");
    const int refId = reference ? reference->id : -1;

    if (!m_projectDocument->addConeOperation(40.0, 0.0, 80.0,
                                              0.0, 0.0, 0.0,
                                              mode, refId,
                                              QStringLiteral("Right"),
                                              QStringLiteral("Left")))
    {
        QMessageBox::critical(this, tr("Rebuild Failed"), m_projectDocument->lastBuildError());
        return;
    }

    const int newId = m_projectDocument->lastOperationId();
    refreshViewport();
    selectOperation(newId);
    statusBar()->showMessage(tr("Added Cone%1").arg(
        reference ? tr(" relative to #%1").arg(refId) : QString()), 4000);
}

void MainWindow::addFilletOperation()
{
    const QVector<OperationEntry> &operations = m_projectDocument->project().operations();
    if (operations.isEmpty())
    {
        QMessageBox::information(this, tr("Add Fillet"), tr("Fillet requires at least one source operation."));
        return;
    }

    // Prefer currently selected operation as source
    int sourceId = -1;
    if (m_projectDocument->findOperation(m_selectedOperationId) != nullptr)
        sourceId = m_selectedOperationId;
    else
        sourceId = operations.constLast().id;

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Create Fillet"));

    auto *layout = new QFormLayout(&dialog);

    auto *sourceComboBox = new QComboBox(&dialog);
    for (const OperationEntry &op : operations)
        sourceComboBox->addItem(QStringLiteral("#%1 %2").arg(op.id).arg(op.label), op.id);

    const int sourceIndex = sourceComboBox->findData(sourceId);
    sourceComboBox->setCurrentIndex(sourceIndex >= 0 ? sourceIndex : sourceComboBox->count() - 1);

    auto *radiusSpinBox = new QDoubleSpinBox(&dialog);
    radiusSpinBox->setDecimals(2);
    radiusSpinBox->setRange(0.1, 1000.0);
    radiusSpinBox->setSingleStep(1.0);
    radiusSpinBox->setValue(5.0);

    layout->addRow(tr("Source"), sourceComboBox);
    layout->addRow(tr("Radius"), radiusSpinBox);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
        return;

    const int chosenSourceId = sourceComboBox->currentData().toInt();
    const double radius = radiusSpinBox->value();

    if (!m_projectDocument->addFilletOperation(chosenSourceId, radius))
    {
        QMessageBox::critical(this, tr("Rebuild Failed"), m_projectDocument->lastBuildError());
        return;
    }

    const int newId = m_projectDocument->lastOperationId();
    refreshViewport();
    selectOperation(newId);
    statusBar()->showMessage(tr("Added Fillet R=%1 on #%2").arg(radius).arg(chosenSourceId), 4000);
}

void MainWindow::addFuseOperation()
{
    const QVector<OperationEntry> sources = m_projectDocument->project().operations();
    if (sources.size() < 2)
    {
        QMessageBox::information(this, tr("Add Fuse"), tr("Fuse requires at least two source operations."));
        return;
    }

    BooleanOperationParameters parameters;
    parameters.leftId = sources.at(sources.size() - 2).id;
    parameters.rightId = sources.constLast().id;
    if (!promptBooleanParameters(this, tr("Create Fuse"), sources, &parameters))
        return;

    if (!m_projectDocument->addFuseOperation(parameters.leftId, parameters.rightId))
    {
        QMessageBox::critical(this, tr("Rebuild Failed"), m_projectDocument->lastBuildError());
        return;
    }

    const int newId = m_projectDocument->lastOperationId();
    refreshViewport();
    selectOperation(newId);
    statusBar()->showMessage(tr("Added Fuse #%1 + #%2").arg(parameters.leftId).arg(parameters.rightId), 4000);
}

void MainWindow::addCutOperation()
{
    const QVector<OperationEntry> sources = m_projectDocument->project().operations();
    if (sources.size() < 2)
    {
        QMessageBox::information(this, tr("Add Cut"), tr("Cut requires at least two source operations."));
        return;
    }

    BooleanOperationParameters parameters;
    parameters.leftId = sources.at(sources.size() - 2).id;
    parameters.rightId = sources.constLast().id;
    if (!promptBooleanParameters(this, tr("Create Cut"), sources, &parameters))
        return;

    if (!m_projectDocument->addCutOperation(parameters.leftId, parameters.rightId))
    {
        QMessageBox::critical(this, tr("Rebuild Failed"), m_projectDocument->lastBuildError());
        return;
    }

    const int newId = m_projectDocument->lastOperationId();
    refreshViewport();
    selectOperation(newId);
    statusBar()->showMessage(tr("Added Cut #%1 - #%2").arg(parameters.leftId).arg(parameters.rightId), 4000);
}

void MainWindow::importStep()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this, tr("Import STEP"), QString(), tr("STEP Files (*.step *.stp)"));

    if (filePath.isEmpty())
        return;

    QString errorMessage;
    TopoDS_Shape importedShape;
    if (!StepExchange::importStep(filePath, importedShape, &errorMessage))
    {
        QMessageBox::critical(this, tr("STEP Import Failed"), errorMessage);
        return;
    }

    m_selectedOperationId = -1;
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
        this, tr("Export STEP"), QString(), tr("STEP Files (*.step *.stp)"));

    if (filePath.isEmpty())
        return;

    QString normalizedPath = filePath;
    if (!normalizedPath.endsWith(QLatin1String(".step"), Qt::CaseInsensitive)
        && !normalizedPath.endsWith(QLatin1String(".stp"), Qt::CaseInsensitive))
    {
        normalizedPath += QStringLiteral(".step");
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
        statusBar()->showMessage(tr("Operation #%1 selected — add a new primitive to attach it here.").arg(operationId), 4000);
}

void MainWindow::updateOperationParameter(const int operationId, const QString &name, const QVariant &value)
{
    if (name == QLatin1String("PlacementMode") && value.toString() == QLatin1String("relative"))
    {
        const OperationEntry *operation = m_projectDocument->findOperation(operationId);
        if (operation != nullptr)
        {
            int referenceId = -1;
            for (const OperationEntry &candidate : m_projectDocument->project().operations())
            {
                if (candidate.id == operationId
                    || candidate.type == QLatin1String("fuse")
                    || candidate.type == QLatin1String("cut"))
                    continue;

                referenceId = candidate.id;
                break;
            }

            if (referenceId < 0)
            {
                QMessageBox::information(this,
                                         tr("Relative Placement"),
                                         tr("Relative placement requires another primitive operation as reference."));
                selectOperation(operationId);
                return;
            }

            if (!m_projectDocument->setOperationParameter(operationId, QStringLiteral("ReferenceId"), referenceId))
            {
                QMessageBox::critical(this, tr("Rebuild Failed"), m_projectDocument->lastBuildError());
                return;
            }
        }
    }

    if (!m_projectDocument->setOperationParameter(operationId, name, value))
    {
        QMessageBox::critical(this, tr("Rebuild Failed"), m_projectDocument->lastBuildError());
        return;
    }

    refreshViewport();
    selectOperation(operationId);
    statusBar()->showMessage(tr("Updated %1 for operation #%2").arg(name).arg(operationId), 4000);
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
    if (m_projectDocument->hasShape())
        m_viewport->setShape(m_projectDocument->shape());
    else
        m_viewport->clearShape();

    if (m_selectedOperationId >= 0)
    {
        const TopoDS_Shape operationShape = m_projectDocument->shapeForOperation(m_selectedOperationId);
        m_viewport->setHighlightedShape(operationShape);
        m_viewport->setDisplayedShapeSelected(!operationShape.IsNull());
    }
    else
    {
        m_viewport->setHighlightedShape(TopoDS_Shape());
        m_viewport->setDisplayedShapeSelected(false);
    }
}

void MainWindow::createActions()
{
    m_newProjectAction = new QAction(tr("New Project"), this);
    m_newProjectAction->setShortcut(QKeySequence::New);
    m_newProjectAction->setToolTip(tr("<b>New Project</b> (Ctrl+N)<br>Reset to a default startup box."));

    m_importStepAction = new QAction(tr("Import STEP..."), this);
    m_importStepAction->setShortcut(tr("Ctrl+I"));
    m_importStepAction->setToolTip(tr("<b>Import STEP</b> (Ctrl+I)<br>Load an external .step / .stp file into the scene."));

    m_exportStepAction = new QAction(tr("Export STEP..."), this);
    m_exportStepAction->setShortcut(tr("Ctrl+E"));
    m_exportStepAction->setToolTip(tr("<b>Export STEP</b> (Ctrl+E)<br>Save the current model to a .step file."));

    m_exitAction = new QAction(tr("Exit"), this);
    m_exitAction->setShortcut(QKeySequence::Quit);

    m_addBoxAction = new QAction(tr("Add Box"), this);
    m_addBoxAction->setToolTip(tr(
        "<b>Add Box</b><br>"
        "Adds a rectangular box (120 × 80 × 60 mm).<br><br>"
        "<i>Tip:</i> Select an existing operation first —<br>"
        "the new box will attach to its right face automatically."));

    m_addCylinderAction = new QAction(tr("Add Cylinder"), this);
    m_addCylinderAction->setToolTip(tr(
        "<b>Add Cylinder</b><br>"
        "Adds a cylinder (R=40, H=100 mm).<br><br>"
        "<i>Tip:</i> Select an existing operation first —<br>"
        "the cylinder attaches to its right face automatically.<br>"
        "Ideal for building shaft segments."));

    m_addConeAction = new QAction(tr("Add Cone"), this);
    m_addConeAction->setToolTip(tr(
        "<b>Add Cone</b><br>"
        "Adds a cone (R1=40, R2=0, H=80 mm).<br><br>"
        "<i>Tip:</i> Select an existing operation first —<br>"
        "the cone attaches to its right face automatically.<br>"
        "Use for tapered shaft transitions."));

    m_addFilletAction = new QAction(tr("Add Fillet"), this);
    m_addFilletAction->setToolTip(tr(
        "<b>Add Fillet</b><br>"
        "Rounds all edges of a source operation.<br><br>"
        "<i>Tip:</i> Select the operation to fillet first —<br>"
        "it will be pre-selected in the dialog."));

    m_addFuseAction = new QAction(tr("Add Fuse"), this);
    m_addFuseAction->setToolTip(tr(
        "<b>Add Fuse (Boolean Union)</b><br>"
        "Merges two operations into a single solid.<br><br>"
        "Choose Left and Right operands in the dialog.<br>"
        "Requires at least two existing operations."));

    m_addCutAction = new QAction(tr("Add Cut"), this);
    m_addCutAction->setToolTip(tr(
        "<b>Add Cut (Boolean Subtraction)</b><br>"
        "Subtracts the Right operand from the Left.<br><br>"
        "Choose Left (base) and Right (tool) in the dialog.<br>"
        "Requires at least two existing operations."));

    m_fitViewAction = new QAction(tr("Fit View"), this);
    m_fitViewAction->setToolTip(tr("<b>Fit View</b><br>Zoom to fit the entire model in the viewport."));

    m_wireframeAction = new QAction(tr("Wireframe"), this);
    m_wireframeAction->setToolTip(tr("<b>Wireframe</b><br>Display model as edges only."));

    m_shadedAction = new QAction(tr("Shaded"), this);
    m_shadedAction->setToolTip(tr("<b>Shaded</b><br>Display model as a solid shaded surface."));

    m_frontViewAction = new QAction(tr("Front"), this);
    m_frontViewAction->setToolTip(tr("<b>Front View</b><br>Look at the model from the front (−Y direction)."));

    m_topViewAction = new QAction(tr("Top"), this);
    m_topViewAction->setToolTip(tr("<b>Top View</b><br>Look at the model from above (+Z direction)."));

    m_rightViewAction = new QAction(tr("Right"), this);
    m_rightViewAction->setToolTip(tr("<b>Right View</b><br>Look at the model from the right (+X direction)."));

    m_isometricViewAction = new QAction(tr("Isometric"), this);
    m_isometricViewAction->setToolTip(tr("<b>Isometric View</b><br>Standard isometric perspective."));

    connect(m_newProjectAction, &QAction::triggered, this, &MainWindow::newProject);
    connect(m_addBoxAction, &QAction::triggered, this, &MainWindow::addBoxOperation);
    connect(m_addCylinderAction, &QAction::triggered, this, &MainWindow::addCylinderOperation);
    connect(m_addConeAction, &QAction::triggered, this, &MainWindow::addConeOperation);
    connect(m_addFilletAction, &QAction::triggered, this, &MainWindow::addFilletOperation);
    connect(m_addFuseAction, &QAction::triggered, this, &MainWindow::addFuseOperation);
    connect(m_addCutAction, &QAction::triggered, this, &MainWindow::addCutOperation);
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
    connect(m_propertyDock,
            &PropertyEditorDock::operationParameterEdited,
            this,
            &MainWindow::updateOperationParameter,
            Qt::QueuedConnection);
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
    modelMenu->addAction(m_addConeAction);
    modelMenu->addSeparator();
    modelMenu->addAction(m_addFilletAction);
    modelMenu->addAction(m_addFuseAction);
    modelMenu->addAction(m_addCutAction);
    menuBar()->addMenu(tr("Help"));
}

void MainWindow::createToolBar()
{
    QToolBar *mainToolBar = addToolBar(tr("Main Toolbar"));
    mainToolBar->setMovable(false);
    mainToolBar->setToolTipsVisible(true);
    mainToolBar->addAction(m_newProjectAction);
    mainToolBar->addSeparator();
    mainToolBar->addAction(m_addBoxAction);
    mainToolBar->addAction(m_addCylinderAction);
    mainToolBar->addAction(m_addConeAction);
    mainToolBar->addAction(m_addFilletAction);
    mainToolBar->addAction(m_addFuseAction);
    mainToolBar->addAction(m_addCutAction);
    mainToolBar->addSeparator();
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

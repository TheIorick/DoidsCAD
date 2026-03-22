#include "mainwindow.h"

#include "cadviewport.h"
#include "operationlistdock.h"
#include "projectdocument.h"
#include "propertyeditordock.h"
#include "stepexchange.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <QAction>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QDoubleSpinBox>
#include <QStatusBar>
#include <QToolBar>

namespace
{
struct PrimitiveParameters
{
    double firstSize = 0.0;
    double secondSize = 0.0;
    double thirdSize = 0.0;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    bool pickInViewport = false;
};

struct BooleanOperationParameters
{
    int leftId = -1;
    int rightId = -1;
};

QDoubleSpinBox *createSizeSpinBox(const double value)
{
    auto *spinBox = new QDoubleSpinBox();
    spinBox->setDecimals(2);
    spinBox->setRange(0.1, 1000000.0);
    spinBox->setSingleStep(10.0);
    spinBox->setValue(value);
    return spinBox;
}

QDoubleSpinBox *createPositionSpinBox(const double value)
{
    auto *spinBox = new QDoubleSpinBox();
    spinBox->setDecimals(2);
    spinBox->setRange(-1000000.0, 1000000.0);
    spinBox->setSingleStep(10.0);
    spinBox->setValue(value);
    return spinBox;
}

bool promptPrimitiveParameters(QWidget *parent,
                               const QString &title,
                               const QString &firstLabel,
                               const QString &secondLabel,
                               const QString &thirdLabel,
                               PrimitiveParameters *parameters)
{
    if (parameters == nullptr)
        return false;

    QDialog dialog(parent);
    dialog.setWindowTitle(title);

    auto *layout = new QFormLayout(&dialog);
    auto *firstSpinBox = createSizeSpinBox(parameters->firstSize);
    auto *secondSpinBox = createSizeSpinBox(parameters->secondSize);
    auto *thirdSpinBox = thirdLabel.isEmpty() ? nullptr : createSizeSpinBox(parameters->thirdSize);
    auto *xSpinBox = createPositionSpinBox(parameters->x);
    auto *ySpinBox = createPositionSpinBox(parameters->y);
    auto *zSpinBox = createPositionSpinBox(parameters->z);
    auto *placementModeComboBox = new QComboBox(&dialog);
    placementModeComboBox->addItem(QObject::tr("By Coordinates"), false);
    placementModeComboBox->addItem(QObject::tr("Pick In Viewport"), true);
    placementModeComboBox->setCurrentIndex(parameters->pickInViewport ? 1 : 0);

    layout->addRow(firstLabel, firstSpinBox);
    layout->addRow(secondLabel, secondSpinBox);
    if (thirdSpinBox != nullptr)
        layout->addRow(thirdLabel, thirdSpinBox);
    layout->addRow(QObject::tr("Placement"), placementModeComboBox);
    layout->addRow(QObject::tr("X"), xSpinBox);
    layout->addRow(QObject::tr("Y"), ySpinBox);
    layout->addRow(QObject::tr("Z"), zSpinBox);

    const auto updateCoordinateState = [=]() {
        const bool pickInViewport = placementModeComboBox->currentData().toBool();
        xSpinBox->setEnabled(!pickInViewport);
        ySpinBox->setEnabled(!pickInViewport);
        zSpinBox->setEnabled(!pickInViewport);
    };
    QObject::connect(placementModeComboBox, &QComboBox::currentIndexChanged, &dialog, updateCoordinateState);
    updateCoordinateState();

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addRow(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
        return false;

    parameters->firstSize = firstSpinBox->value();
    parameters->secondSize = secondSpinBox->value();
    parameters->thirdSize = thirdSpinBox != nullptr ? thirdSpinBox->value() : 0.0;
    parameters->x = xSpinBox->value();
    parameters->y = ySpinBox->value();
    parameters->z = zSpinBox->value();
    parameters->pickInViewport = placementModeComboBox->currentData().toBool();
    return true;
}

QVector<OperationEntry> availableBooleanSources(const ProjectModel &projectModel)
{
    QVector<OperationEntry> sources;

    for (const OperationEntry &operation : projectModel.operations())
    {
        if (operation.type == QLatin1String("fuse") || operation.type == QLatin1String("cut"))
            continue;

        sources.append(operation);
    }

    return sources;
}

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
    PrimitiveParameters parameters;
    parameters.firstSize = 120.0 + index * 20.0;
    parameters.secondSize = 80.0 + index * 10.0;
    parameters.thirdSize = 60.0 + index * 5.0;
    parameters.x = index * 160.0;

    if (!promptPrimitiveParameters(this,
                                   tr("Create Box"),
                                   tr("Length"),
                                   tr("Width"),
                                   tr("Height"),
                                   &parameters))
        return;

    if (parameters.pickInViewport)
    {
        m_pendingPrimitiveCreation.type = PendingPrimitiveType::Box;
        m_pendingPrimitiveCreation.firstSize = parameters.firstSize;
        m_pendingPrimitiveCreation.secondSize = parameters.secondSize;
        m_pendingPrimitiveCreation.thirdSize = parameters.thirdSize;
        m_viewport->setPlacementPreviewShape(
            BRepPrimAPI_MakeBox(parameters.firstSize, parameters.secondSize, parameters.thirdSize).Shape());
        m_viewport->startPlacementPick();
        statusBar()->showMessage(tr("Click in viewport to place the box on Z=0 plane. Right click cancels."), 8000);
        return;
    }

    if (!m_projectDocument->addBoxOperation(parameters.firstSize,
                                            parameters.secondSize,
                                            parameters.thirdSize,
                                            parameters.x,
                                            parameters.y,
                                            parameters.z))
    {
        QMessageBox::critical(this, tr("Rebuild Failed"), m_projectDocument->lastBuildError());
        return;
    }

    refreshViewport();
    statusBar()->showMessage(tr("Added Box operation: %1 x %2 x %3 at (%4, %5, %6)")
                                 .arg(parameters.firstSize)
                                 .arg(parameters.secondSize)
                                 .arg(parameters.thirdSize)
                                 .arg(parameters.x)
                                 .arg(parameters.y)
                                 .arg(parameters.z),
                             5000);
}

void MainWindow::addCylinderOperation()
{
    const int index = m_projectDocument->project().operationCount();
    PrimitiveParameters parameters;
    parameters.firstSize = 40.0 + index * 5.0;
    parameters.secondSize = 100.0 + index * 15.0;
    parameters.x = index * 160.0;

    if (!promptPrimitiveParameters(this,
                                   tr("Create Cylinder"),
                                   tr("Radius"),
                                   tr("Height"),
                                   QString(),
                                   &parameters))
        return;

    if (parameters.pickInViewport)
    {
        m_pendingPrimitiveCreation.type = PendingPrimitiveType::Cylinder;
        m_pendingPrimitiveCreation.firstSize = parameters.firstSize;
        m_pendingPrimitiveCreation.secondSize = parameters.secondSize;
        m_pendingPrimitiveCreation.thirdSize = 0.0;
        m_viewport->setPlacementPreviewShape(
            BRepPrimAPI_MakeCylinder(parameters.firstSize, parameters.secondSize).Shape());
        m_viewport->startPlacementPick();
        statusBar()->showMessage(tr("Click in viewport to place the cylinder on Z=0 plane. Right click cancels."), 8000);
        return;
    }

    if (!m_projectDocument->addCylinderOperation(parameters.firstSize,
                                                 parameters.secondSize,
                                                 parameters.x,
                                                 parameters.y,
                                                 parameters.z))
    {
        QMessageBox::critical(this, tr("Rebuild Failed"), m_projectDocument->lastBuildError());
        return;
    }

    refreshViewport();
    statusBar()->showMessage(tr("Added Cylinder operation: R%1 H%2 at (%3, %4, %5)")
                                 .arg(parameters.firstSize)
                                 .arg(parameters.secondSize)
                                 .arg(parameters.x)
                                 .arg(parameters.y)
                                 .arg(parameters.z),
                             5000);
}

void MainWindow::addFuseOperation()
{
    const QVector<OperationEntry> sources = availableBooleanSources(m_projectDocument->project());
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

    refreshViewport();
    statusBar()->showMessage(tr("Added Fuse operation for #%1 and #%2")
                                 .arg(parameters.leftId)
                                 .arg(parameters.rightId),
                             5000);
}

void MainWindow::addCutOperation()
{
    const QVector<OperationEntry> sources = availableBooleanSources(m_projectDocument->project());
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

    refreshViewport();
    statusBar()->showMessage(tr("Added Cut operation for #%1 and #%2")
                                 .arg(parameters.leftId)
                                 .arg(parameters.rightId),
                             5000);
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

void MainWindow::finishViewportPlacement(const double x, const double y, const double z)
{
    createPrimitiveAtPickedPoint(x, y, z);
}

void MainWindow::cancelViewportPlacement()
{
    m_pendingPrimitiveCreation = PendingPrimitiveCreation();
    statusBar()->showMessage(tr("Viewport placement canceled."), 4000);
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

void MainWindow::createPrimitiveAtPickedPoint(const double x, const double y, const double z)
{
    const PendingPrimitiveCreation pending = m_pendingPrimitiveCreation;
    m_pendingPrimitiveCreation = PendingPrimitiveCreation();

    bool success = false;
    QString statusMessage;
    if (pending.type == PendingPrimitiveType::Box)
    {
        success = m_projectDocument->addBoxOperation(pending.firstSize,
                                                     pending.secondSize,
                                                     pending.thirdSize,
                                                     x,
                                                     y,
                                                     z);
        statusMessage = tr("Added Box operation: %1 x %2 x %3 at (%4, %5, %6)")
                            .arg(pending.firstSize)
                            .arg(pending.secondSize)
                            .arg(pending.thirdSize)
                            .arg(x)
                            .arg(y)
                            .arg(z);
    }
    else if (pending.type == PendingPrimitiveType::Cylinder)
    {
        success = m_projectDocument->addCylinderOperation(pending.firstSize,
                                                          pending.secondSize,
                                                          x,
                                                          y,
                                                          z);
        statusMessage = tr("Added Cylinder operation: R%1 H%2 at (%3, %4, %5)")
                            .arg(pending.firstSize)
                            .arg(pending.secondSize)
                            .arg(x)
                            .arg(y)
                            .arg(z);
    }
    else
    {
        return;
    }

    if (!success)
    {
        QMessageBox::critical(this, tr("Rebuild Failed"), m_projectDocument->lastBuildError());
        return;
    }

    refreshViewport();
    statusBar()->showMessage(statusMessage, 5000);
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
    m_addCutAction = new QAction(tr("Add Cut"), this);

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
    connect(m_viewport, &CadViewport::placementPointPicked, this, &MainWindow::finishViewportPlacement);
    connect(m_viewport, &CadViewport::placementCanceled, this, &MainWindow::cancelViewportPlacement);
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
    modelMenu->addAction(m_addFuseAction);
    modelMenu->addAction(m_addCutAction);
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
    mainToolBar->addAction(m_addCutAction);
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

#include "propertyeditordock.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QLabel>
#include <QStringList>
#include <QVBoxLayout>

namespace
{
const QStringList &placementPointOptions()
{
    static const QStringList options = {
        QStringLiteral("Center"),
        QStringLiteral("Left"),
        QStringLiteral("Right"),
        QStringLiteral("Front"),
        QStringLiteral("Back"),
        QStringLiteral("Bottom"),
        QStringLiteral("Top")
    };
    return options;
}

QLabel *createSectionTitle(const QString &text)
{
    auto *label = new QLabel(text);
    QFont font = label->font();
    font.setBold(true);
    label->setFont(font);
    return label;
}

QDoubleSpinBox *createDimensionSpinBox(const double value)
{
    auto *spinBox = new QDoubleSpinBox();
    spinBox->setDecimals(2);
    spinBox->setRange(0.1, 1000000.0);
    spinBox->setValue(value);
    spinBox->setSingleStep(10.0);
    return spinBox;
}

QDoubleSpinBox *createPositionSpinBox(const double value)
{
    auto *spinBox = new QDoubleSpinBox();
    spinBox->setDecimals(2);
    spinBox->setRange(-1000000.0, 1000000.0);
    spinBox->setValue(value);
    spinBox->setSingleStep(10.0);
    return spinBox;
}

double parameterValue(const OperationEntry &operation, const QString &name, const double fallback)
{
    for (const OperationParameter &parameter : operation.parameters)
    {
        if (parameter.name == name)
            return parameter.value.toDouble();
    }

    return fallback;
}

QString parameterText(const OperationEntry &operation, const QString &name)
{
    for (const OperationParameter &parameter : operation.parameters)
    {
        if (parameter.name == name)
            return parameter.value.toString();
    }

    return QString();
}
}

PropertyEditorDock::PropertyEditorDock(QWidget *parent)
    : QDockWidget(tr("Properties"), parent)
    , m_containerWidget(new QWidget(this))
    , m_rootLayout(new QVBoxLayout())
    , m_selectionValueLabel(new QLabel(tr("None"), this))
    , m_detailsWidget(nullptr)
    , m_isUpdating(false)
{
    m_rootLayout->setContentsMargins(12, 12, 12, 12);
    m_rootLayout->setSpacing(12);

    auto *selectionTitle = createSectionTitle(tr("Selection"));
    m_rootLayout->addWidget(selectionTitle);
    m_rootLayout->addWidget(m_selectionValueLabel);

    auto *separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    m_rootLayout->addWidget(separator);

    replaceDetailsWidget(createPlaceholderWidget());

    m_rootLayout->addStretch();
    m_containerWidget->setLayout(m_rootLayout);
    setWidget(m_containerWidget);
}

void PropertyEditorDock::setSelectionDescription(const QString &description)
{
    m_selectionValueLabel->setText(description);
}

void PropertyEditorDock::showOperationDetails(const OperationEntry *operation, const QVector<OperationEntry> &allOperations)
{
    m_isUpdating = true;
    replaceDetailsWidget(operation == nullptr ? createPlaceholderWidget()
                                             : createOperationWidget(operation, allOperations));
    m_isUpdating = false;
}

QWidget *PropertyEditorDock::createPlaceholderWidget() const
{
    auto *widget = new QWidget();
    auto *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);
    layout->addWidget(createSectionTitle(tr("Document")));
    layout->addWidget(new QLabel(tr("Format: STEP")));
    layout->addWidget(new QLabel(tr("Compiler: MSVC")));
    layout->addWidget(new QLabel(tr("Viewer: OpenCascade")));
    layout->addStretch();
    return widget;
}

QWidget *PropertyEditorDock::createOperationWidget(const OperationEntry *operation,
                                                   const QVector<OperationEntry> &allOperations)
{
    auto *widget = new QWidget();
    auto *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    layout->addWidget(createSectionTitle(tr("Operation")));

    auto *metaForm = new QFormLayout();
    metaForm->setContentsMargins(0, 0, 0, 0);
    metaForm->addRow(tr("Name"), new QLabel(operation->label));
    metaForm->addRow(tr("Type"), new QLabel(operation->type));
    metaForm->addRow(tr("State"), new QLabel(operation->state));
    layout->addLayout(metaForm);

    if (operation->type == QLatin1String("box") || operation->type == QLatin1String("cylinder"))
    {
        const auto addNumericRow = [this, operation](QFormLayout *form,
                                                     const QString &label,
                                                     const QString &parameterName,
                                                     const double value,
                                                     const bool isPosition) {
            QDoubleSpinBox *spinBox = isPosition ? createPositionSpinBox(value) : createDimensionSpinBox(value);
            connect(spinBox,
                    &QDoubleSpinBox::valueChanged,
                    this,
                    [this, operationId = operation->id, parameterName](double newValue) {
                        if (!m_isUpdating)
                            emit operationParameterEdited(operationId, parameterName, newValue);
                    });
            form->addRow(label, spinBox);
        };

        const auto addTextCombo = [this, operation](QFormLayout *form,
                                                    const QString &label,
                                                    const QString &parameterName,
                                                    const QStringList &options,
                                                    const QString &currentValue) {
            auto *comboBox = new QComboBox();
            for (const QString &option : options)
                comboBox->addItem(option, option);

            const int comboIndex = comboBox->findData(currentValue);
            if (comboIndex >= 0)
                comboBox->setCurrentIndex(comboIndex);

            connect(comboBox,
                    &QComboBox::currentIndexChanged,
                    this,
                    [this, operationId = operation->id, parameterName, comboBox](int) {
                        if (!m_isUpdating)
                            emit operationParameterEdited(operationId, parameterName, comboBox->currentData());
                    });
            form->addRow(label, comboBox);
        };

        layout->addWidget(createSectionTitle(tr("Geometry")));
        auto *geometryForm = new QFormLayout();
        geometryForm->setContentsMargins(0, 0, 0, 0);

        if (operation->type == QLatin1String("box"))
        {
            addNumericRow(geometryForm, tr("Length"), QStringLiteral("Length"), parameterValue(*operation, QStringLiteral("Length"), 120.0), false);
            addNumericRow(geometryForm, tr("Width"), QStringLiteral("Width"), parameterValue(*operation, QStringLiteral("Width"), 80.0), false);
            addNumericRow(geometryForm, tr("Height"), QStringLiteral("Height"), parameterValue(*operation, QStringLiteral("Height"), 60.0), false);
        }
        else
        {
            addNumericRow(geometryForm, tr("Radius"), QStringLiteral("Radius"), parameterValue(*operation, QStringLiteral("Radius"), 40.0), false);
            addNumericRow(geometryForm, tr("Height"), QStringLiteral("Height"), parameterValue(*operation, QStringLiteral("Height"), 100.0), false);
        }

        layout->addLayout(geometryForm);

        layout->addWidget(createSectionTitle(tr("Placement")));
        auto *placementForm = new QFormLayout();
        placementForm->setContentsMargins(0, 0, 0, 0);
        const QString placementMode = parameterText(*operation, QStringLiteral("PlacementMode")).toLower();
        addTextCombo(placementForm,
                     tr("Mode"),
                     QStringLiteral("PlacementMode"),
                     {QStringLiteral("absolute"), QStringLiteral("relative")},
                     placementMode.isEmpty() ? QStringLiteral("absolute") : placementMode);

        if (placementMode == QLatin1String("relative"))
        {
            auto *referenceComboBox = new QComboBox();
            for (const OperationEntry &candidate : allOperations)
            {
                if (candidate.id == operation->id
                    || candidate.type == QLatin1String("fuse")
                    || candidate.type == QLatin1String("cut"))
                    continue;

                referenceComboBox->addItem(QStringLiteral("#%1 %2").arg(candidate.id).arg(candidate.label), candidate.id);
            }

            const int currentReferenceId = static_cast<int>(parameterValue(*operation, QStringLiteral("ReferenceId"), -1));
            const int comboIndex = referenceComboBox->findData(currentReferenceId);
            if (comboIndex >= 0)
                referenceComboBox->setCurrentIndex(comboIndex);

            connect(referenceComboBox,
                    &QComboBox::currentIndexChanged,
                    this,
                    [this, operationId = operation->id, referenceComboBox](int) {
                        if (!m_isUpdating)
                            emit operationParameterEdited(operationId, QStringLiteral("ReferenceId"), referenceComboBox->currentData());
                    });
            placementForm->addRow(tr("Reference"), referenceComboBox);

            addTextCombo(placementForm,
                         tr("Reference Point"),
                         QStringLiteral("ReferencePoint"),
                         placementPointOptions(),
                         parameterText(*operation, QStringLiteral("ReferencePoint")));
            addTextCombo(placementForm,
                         tr("Self Point"),
                         QStringLiteral("SelfPoint"),
                         placementPointOptions(),
                         parameterText(*operation, QStringLiteral("SelfPoint")));
            addNumericRow(placementForm, tr("Offset X"), QStringLiteral("X"), parameterValue(*operation, QStringLiteral("X"), 0.0), true);
            addNumericRow(placementForm, tr("Offset Y"), QStringLiteral("Y"), parameterValue(*operation, QStringLiteral("Y"), 0.0), true);
            addNumericRow(placementForm, tr("Offset Z"), QStringLiteral("Z"), parameterValue(*operation, QStringLiteral("Z"), 0.0), true);
        }
        else
        {
            addNumericRow(placementForm, tr("X"), QStringLiteral("X"), parameterValue(*operation, QStringLiteral("X"), 0.0), true);
            addNumericRow(placementForm, tr("Y"), QStringLiteral("Y"), parameterValue(*operation, QStringLiteral("Y"), 0.0), true);
            addNumericRow(placementForm, tr("Z"), QStringLiteral("Z"), parameterValue(*operation, QStringLiteral("Z"), 0.0), true);
        }
        layout->addLayout(placementForm);
    }
    else if (operation->type == QLatin1String("fuse") || operation->type == QLatin1String("cut"))
    {
        layout->addWidget(createSectionTitle(tr("Operands")));
        auto *operandForm = new QFormLayout();
        operandForm->setContentsMargins(0, 0, 0, 0);

        const auto addOperationCombo = [this, operation, &allOperations, operandForm](const QString &label,
                                                                                       const QString &parameterName) {
            auto *comboBox = new QComboBox();
            for (const OperationEntry &candidate : allOperations)
            {
                if (candidate.id == operation->id
                    || candidate.type == QLatin1String("fuse")
                    || candidate.type == QLatin1String("cut"))
                    continue;

                comboBox->addItem(QStringLiteral("#%1 %2").arg(candidate.id).arg(candidate.label), candidate.id);
            }

            const int currentId = static_cast<int>(parameterValue(*operation, parameterName, -1));
            const int comboIndex = comboBox->findData(currentId);
            if (comboIndex >= 0)
                comboBox->setCurrentIndex(comboIndex);

            connect(comboBox,
                    &QComboBox::currentIndexChanged,
                    this,
                    [this, operationId = operation->id, parameterName, comboBox](int) {
                        if (!m_isUpdating)
                            emit operationParameterEdited(operationId, parameterName, comboBox->currentData());
                    });

            operandForm->addRow(label, comboBox);
        };

        addOperationCombo(tr("Left"), QStringLiteral("LeftId"));
        addOperationCombo(tr("Right"), QStringLiteral("RightId"));
        layout->addLayout(operandForm);
    }
    else if (operation->type == QLatin1String("import_step"))
    {
        layout->addWidget(createSectionTitle(tr("Source")));
        for (const OperationParameter &parameter : operation->parameters)
            layout->addWidget(new QLabel(QStringLiteral("%1: %2").arg(parameter.name, parameter.value.toString())));
    }

    layout->addStretch();
    return widget;
}

void PropertyEditorDock::replaceDetailsWidget(QWidget *widget)
{
    if (m_detailsWidget != nullptr)
    {
        m_rootLayout->removeWidget(m_detailsWidget);
        delete m_detailsWidget;
    }

    m_detailsWidget = widget;
    m_rootLayout->insertWidget(3, m_detailsWidget);
}

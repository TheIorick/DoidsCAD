#include "cadviewport.h"

#include "occtwindow.h"

#include <AIS_DisplayMode.hxx>
#include <AIS_Shape.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <Graphic3d_GraphicDriver.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <Quantity_Color.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QPaintEngine>
#include <QResizeEvent>
#include <QScreen>
#include <QWheelEvent>

namespace
{
Aspect_VKeyMouse toAspectMouseButtons(const Qt::MouseButtons buttons)
{
    Aspect_VKeyMouse result = Aspect_VKeyMouse_NONE;

    if (buttons.testFlag(Qt::LeftButton))
        result |= Aspect_VKeyMouse_LeftButton;
    if (buttons.testFlag(Qt::MiddleButton))
        result |= Aspect_VKeyMouse_MiddleButton;
    if (buttons.testFlag(Qt::RightButton))
        result |= Aspect_VKeyMouse_RightButton;

    return result;
}

Aspect_VKeyFlags toAspectModifiers(const Qt::KeyboardModifiers modifiers)
{
    Aspect_VKeyFlags result = Aspect_VKeyFlags_NONE;

    if (modifiers.testFlag(Qt::ShiftModifier))
        result |= Aspect_VKeyFlags_SHIFT;
    if (modifiers.testFlag(Qt::ControlModifier))
        result |= Aspect_VKeyFlags_CTRL;
    if (modifiers.testFlag(Qt::AltModifier))
        result |= Aspect_VKeyFlags_ALT;

    return result;
}

Graphic3d_Vec2i devicePosition(const QPointF &position)
{
    const QList<QScreen *> screens = QGuiApplication::screens();
    const qreal scaleFactor = screens.isEmpty() ? 1.0 : screens.constFirst()->devicePixelRatio();

    return Graphic3d_Vec2i(qRound(position.x() * scaleFactor), qRound(position.y() * scaleFactor));
}

QString shapeTypeToText(const TopAbs_ShapeEnum shapeType)
{
    switch (shapeType)
    {
    case TopAbs_COMPOUND:
        return QObject::tr("Compound");
    case TopAbs_COMPSOLID:
        return QObject::tr("Composite solid");
    case TopAbs_SOLID:
        return QObject::tr("Solid");
    case TopAbs_SHELL:
        return QObject::tr("Shell");
    case TopAbs_FACE:
        return QObject::tr("Face");
    case TopAbs_WIRE:
        return QObject::tr("Wire");
    case TopAbs_EDGE:
        return QObject::tr("Edge");
    case TopAbs_VERTEX:
        return QObject::tr("Vertex");
    case TopAbs_SHAPE:
        return QObject::tr("Shape");
    default:
        return QObject::tr("Unknown");
    }
}
}

CadViewport::CadViewport(QWidget *parent)
    : QWidget(parent, Qt::MSWindowsOwnDC)
    , m_isDragging(false)
{
    setContentsMargins(0, 0, 0, 0);
    setMouseTracking(true);
    setAutoFillBackground(false);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setFocusPolicy(Qt::StrongFocus);
    setBackgroundRole(QPalette::NoRole);

    initializeViewer();
}

QPaintEngine *CadViewport::paintEngine() const
{
    return nullptr;
}

void CadViewport::setShape(const TopoDS_Shape &shape)
{
    if (m_context.IsNull())
        return;

    if (!m_shapePresentation.IsNull())
        m_context->Remove(m_shapePresentation, Standard_False);

    clearHighlightPresentation();

    m_currentShape = shape;
    m_shapePresentation.Nullify();

    if (!shape.IsNull())
    {
        m_shapePresentation = new AIS_Shape(shape);
        m_context->Display(m_shapePresentation, AIS_Shaded, 0, Standard_False);
    }

    m_context->ClearSelected(Standard_False);
    updateSelectionDescription();

    if (!shape.IsNull())
        fitAll();
    else
        update();
}

void CadViewport::clearShape()
{
    setShape(TopoDS_Shape());
}

void CadViewport::fitAll()
{
    if (m_view.IsNull())
        return;

    m_view->FitAll(0.01, Standard_False);
    m_view->ZFitAll(0.01);
    update();
}

void CadViewport::setWireframeMode()
{
    if (m_context.IsNull() || m_shapePresentation.IsNull())
        return;

    m_context->SetDisplayMode(m_shapePresentation, AIS_WireFrame, Standard_True);
}

void CadViewport::setShadedMode()
{
    if (m_context.IsNull() || m_shapePresentation.IsNull())
        return;

    m_context->SetDisplayMode(m_shapePresentation, AIS_Shaded, Standard_True);
}

void CadViewport::setFrontView()
{
    setViewOrientation(V3d_TypeOfOrientation_Zup_Front);
}

void CadViewport::setTopView()
{
    setViewOrientation(V3d_TypeOfOrientation_Zup_Top);
}

void CadViewport::setRightView()
{
    setViewOrientation(V3d_TypeOfOrientation_Zup_Right);
}

void CadViewport::setIsometricView()
{
    setViewOrientation(V3d_TypeOfOrientation_Zup_AxoRight);
}

void CadViewport::setDisplayedShapeSelected(const bool selected)
{
    if (m_context.IsNull() || m_shapePresentation.IsNull())
        return;

    if (selected)
        m_context->SetSelected(m_shapePresentation, Standard_False);
    else
        m_context->ClearSelected(Standard_False);

    updateSelectionDescription();
    update();
}

void CadViewport::setHighlightedShape(const TopoDS_Shape &shape)
{
    if (m_context.IsNull())
        return;

    clearHighlightPresentation();

    if (shape.IsNull())
    {
        update();
        return;
    }

    m_highlightPresentation = new AIS_Shape(shape);
    m_highlightPresentation->SetColor(Quantity_NOC_RED);
    m_context->Display(m_highlightPresentation, AIS_WireFrame, 1, Standard_False);
    update();
}

void CadViewport::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (!m_view.IsNull())
        m_view->MustBeResized();
}

void CadViewport::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    if (m_view.IsNull())
        return;

    m_view->InvalidateImmediate();
    FlushViewEvents(m_context, m_view, Standard_True);
}

void CadViewport::mousePressEvent(QMouseEvent *event)
{
    m_lastPressPosition = event->position();
    m_isDragging = false;

    if (UpdateMouseButtons(devicePosition(event->position()),
                           toAspectMouseButtons(event->buttons()),
                           toAspectModifiers(event->modifiers()),
                           false))
        update();
}

void CadViewport::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton
        && !m_context.IsNull()
        && !m_view.IsNull()
        && !m_isDragging)
    {
        const Graphic3d_Vec2i mousePosition = devicePosition(event->position());
        m_context->MoveTo(mousePosition.x(), mousePosition.y(), m_view, Standard_False);
        m_context->SelectDetected(AIS_SelectionScheme_Replace);
        updateSelectionDescription();
        update();
    }

    if (UpdateMouseButtons(devicePosition(event->position()),
                           toAspectMouseButtons(event->buttons()),
                           toAspectModifiers(event->modifiers()),
                           false))
        update();

    QWidget::mouseReleaseEvent(event);
}

void CadViewport::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_isDragging && (event->position() - m_lastPressPosition).manhattanLength() >= 3.0)
        m_isDragging = true;

    if (event->buttons() == Qt::NoButton && !m_context.IsNull() && !m_view.IsNull())
        m_context->MoveTo(devicePosition(event->position()).x(), devicePosition(event->position()).y(), m_view, Standard_False);

    if (UpdateMousePosition(devicePosition(event->position()),
                            toAspectMouseButtons(event->buttons()),
                            toAspectModifiers(event->modifiers()),
                            false))
        update();
}

void CadViewport::wheelEvent(QWheelEvent *event)
{
    if (UpdateZoom(Aspect_ScrollDelta(devicePosition(event->position()), event->angleDelta().y() / 8)))
        update();
}

void CadViewport::initializeViewer()
{
    Handle(Aspect_DisplayConnection) displayConnection = new Aspect_DisplayConnection();
    Handle(OpenGl_GraphicDriver) graphicDriver = new OpenGl_GraphicDriver(displayConnection);

    m_viewer = new V3d_Viewer(graphicDriver);
    m_viewer->SetDefaultLights();
    m_viewer->SetLightOn();

    m_context = new AIS_InteractiveContext(m_viewer);
    m_view = m_context->CurrentViewer()->CreateView();

    Handle(OcctWindow) window = new OcctWindow(this);
    m_view->SetWindow(window);

    if (!window->IsMapped())
        window->Map();

    SetAllowRotation(true);
    m_view->SetBackgroundColor(Quantity_Color(0.95, 0.95, 0.97, Quantity_TOC_RGB));
    m_view->TriedronDisplay(Aspect_TOTP_RIGHT_LOWER, Quantity_NOC_GRAY40, 0.08, V3d_ZBUFFER);
    m_view->MustBeResized();
}

void CadViewport::setViewOrientation(const V3d_TypeOfOrientation orientation)
{
    if (m_view.IsNull())
        return;

    m_view->SetProj(orientation, Standard_False);
    fitAll();
}

void CadViewport::clearHighlightPresentation()
{
    if (!m_context.IsNull() && !m_highlightPresentation.IsNull())
        m_context->Remove(m_highlightPresentation, Standard_False);

    m_highlightPresentation.Nullify();
}

void CadViewport::updateSelectionDescription()
{
    QString description = tr("None");

    if (!m_context.IsNull() && m_context->NbSelected() > 0)
    {
        m_context->InitSelected();
        if (m_context->MoreSelected())
        {
            const Handle(AIS_InteractiveObject) selectedObject = m_context->SelectedInteractive();
            if (!selectedObject.IsNull())
            {
                const Handle(AIS_Shape) selectedShape = Handle(AIS_Shape)::DownCast(selectedObject);
                if (!selectedShape.IsNull())
                    description = shapeTypeToText(selectedShape->Shape().ShapeType());
                else
                    description = tr("Interactive object");
            }
        }
    }

    emit selectionDescriptionChanged(description);
}

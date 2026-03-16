#include "cadviewport.h"

#include "occtwindow.h"

#include <AIS_DisplayMode.hxx>
#include <AIS_Shape.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <Interface_Static.hxx>
#include <Graphic3d_GraphicDriver.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <STEPControl_Reader.hxx>
#include <STEPControl_Writer.hxx>
#include <Standard_Failure.hxx>
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
}

CadViewport::CadViewport(QWidget *parent)
    : QWidget(parent, Qt::MSWindowsOwnDC)
{
    setContentsMargins(0, 0, 0, 0);
    setMouseTracking(true);
    setAutoFillBackground(false);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setFocusPolicy(Qt::StrongFocus);
    setBackgroundRole(QPalette::NoRole);

    initializeViewer();
    displayStartupShape();
}

QPaintEngine *CadViewport::paintEngine() const
{
    return nullptr;
}

bool CadViewport::importStep(const QString &filePath, QString *errorMessage)
{
    STEPControl_Reader reader;
    const IFSelect_ReturnStatus readStatus = reader.ReadFile(filePath.toStdString().c_str());

    if (readStatus != IFSelect_RetDone)
    {
        if (errorMessage != nullptr)
            *errorMessage = tr("OpenCascade could not read the STEP file.");

        return false;
    }

    const Standard_Integer roots = reader.TransferRoots();
    if (roots <= 0)
    {
        if (errorMessage != nullptr)
            *errorMessage = tr("The STEP file does not contain transferable shapes.");

        return false;
    }

    const TopoDS_Shape importedShape = reader.OneShape();
    if (importedShape.IsNull())
    {
        if (errorMessage != nullptr)
            *errorMessage = tr("The imported STEP model is empty.");

        return false;
    }

    displayShape(importedShape);
    return true;
}

bool CadViewport::exportStep(const QString &filePath, QString *errorMessage) const
{
    if (m_currentShape.IsNull())
    {
        if (errorMessage != nullptr)
            *errorMessage = tr("There is no shape to export.");

        return false;
    }

    try
    {
        STEPControl_Writer writer;
        Interface_Static::SetCVal("write.step.schema", "AP203");

        const IFSelect_ReturnStatus transferStatus = writer.Transfer(m_currentShape, STEPControl_AsIs);
        if (transferStatus != IFSelect_RetDone)
        {
            if (errorMessage != nullptr)
                *errorMessage = tr("OpenCascade could not prepare the shape for STEP export.");

            return false;
        }

        const IFSelect_ReturnStatus writeStatus = writer.Write(filePath.toStdString().c_str());
        if (writeStatus != IFSelect_RetDone)
        {
            if (errorMessage != nullptr)
                *errorMessage = tr("OpenCascade could not write the STEP file.");

            return false;
        }
    }
    catch (const Standard_Failure &failure)
    {
        if (errorMessage != nullptr)
            *errorMessage = QString::fromLocal8Bit(failure.GetMessageString());

        return false;
    }

    return true;
}

bool CadViewport::hasShape() const
{
    return !m_currentShape.IsNull();
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
    if (UpdateMouseButtons(devicePosition(event->position()),
                           toAspectMouseButtons(event->buttons()),
                           toAspectModifiers(event->modifiers()),
                           false))
        update();
}

void CadViewport::mouseReleaseEvent(QMouseEvent *event)
{
    if (UpdateMouseButtons(devicePosition(event->position()),
                           toAspectMouseButtons(event->buttons()),
                           toAspectModifiers(event->modifiers()),
                           false))
        update();

    QWidget::mouseReleaseEvent(event);
}

void CadViewport::mouseMoveEvent(QMouseEvent *event)
{
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

void CadViewport::displayStartupShape()
{
    displayShape(BRepPrimAPI_MakeBox(120.0, 80.0, 60.0).Shape());
}

void CadViewport::displayShape(const TopoDS_Shape &shape)
{
    if (!m_shapePresentation.IsNull())
        m_context->Remove(m_shapePresentation, Standard_False);

    m_currentShape = shape;
    m_shapePresentation = new AIS_Shape(shape);
    m_context->Display(m_shapePresentation, AIS_Shaded, 0, Standard_False);
    fitAll();
}

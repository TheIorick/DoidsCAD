#include "occtwindow.h"

#include <QGuiApplication>
#include <QScreen>

IMPLEMENT_STANDARD_RTTIEXT(OcctWindow, Aspect_Window)

OcctWindow::OcctWindow(QWidget *widget, const Quantity_NameOfColor backColor)
    : m_widget(widget)
    , m_xLeft(widget->rect().left())
    , m_yTop(widget->rect().top())
    , m_xRight(widget->rect().right())
    , m_yBottom(widget->rect().bottom())
{
    SetBackground(backColor);
}

Aspect_Drawable OcctWindow::NativeHandle() const
{
    return reinterpret_cast<Aspect_Drawable>(m_widget->winId());
}

Aspect_Drawable OcctWindow::NativeParentHandle() const
{
    QWidget *parentWidget = m_widget->parentWidget();
    return parentWidget ? reinterpret_cast<Aspect_Drawable>(parentWidget->winId()) : 0;
}

Aspect_TypeOfResize OcctWindow::DoResize()
{
    Aspect_TypeOfResize mode = Aspect_TOR_UNKNOWN;

    if (!m_widget->isMinimized())
    {
        int mask = 0;

        if (Abs(m_widget->rect().left() - m_xLeft) > 2)
            mask |= 1;
        if (Abs(m_widget->rect().right() - m_xRight) > 2)
            mask |= 2;
        if (Abs(m_widget->rect().top() - m_yTop) > 2)
            mask |= 4;
        if (Abs(m_widget->rect().bottom() - m_yBottom) > 2)
            mask |= 8;

        switch (mask)
        {
        case 0:
            mode = Aspect_TOR_NO_BORDER;
            break;
        case 1:
            mode = Aspect_TOR_LEFT_BORDER;
            break;
        case 2:
            mode = Aspect_TOR_RIGHT_BORDER;
            break;
        case 4:
            mode = Aspect_TOR_TOP_BORDER;
            break;
        case 5:
            mode = Aspect_TOR_LEFT_AND_TOP_BORDER;
            break;
        case 6:
            mode = Aspect_TOR_TOP_AND_RIGHT_BORDER;
            break;
        case 8:
            mode = Aspect_TOR_BOTTOM_BORDER;
            break;
        case 9:
            mode = Aspect_TOR_BOTTOM_AND_LEFT_BORDER;
            break;
        case 10:
            mode = Aspect_TOR_RIGHT_AND_BOTTOM_BORDER;
            break;
        default:
            break;
        }

        m_xLeft = m_widget->rect().left();
        m_xRight = m_widget->rect().right();
        m_yTop = m_widget->rect().top();
        m_yBottom = m_widget->rect().bottom();
    }

    return mode;
}

Standard_Boolean OcctWindow::IsMapped() const
{
    return !(m_widget->isMinimized() || m_widget->isHidden());
}

Standard_Boolean OcctWindow::DoMapping() const
{
    return Standard_True;
}

void OcctWindow::Map() const
{
    m_widget->show();
    m_widget->update();
}

void OcctWindow::Unmap() const
{
    m_widget->hide();
    m_widget->update();
}

void OcctWindow::Position(Standard_Integer &x1,
                          Standard_Integer &y1,
                          Standard_Integer &x2,
                          Standard_Integer &y2) const
{
    x1 = m_widget->rect().left();
    x2 = m_widget->rect().right();
    y1 = m_widget->rect().top();
    y2 = m_widget->rect().bottom();
}

Standard_Real OcctWindow::Ratio() const
{
    const QRect rect = m_widget->rect();
    return Standard_Real(rect.right() - rect.left()) / Standard_Real(rect.bottom() - rect.top());
}

void OcctWindow::Size(Standard_Integer &width, Standard_Integer &height) const
{
    const QList<QScreen *> screens = QGuiApplication::screens();
    const QRect rect = m_widget->rect();
    const qreal scaleFactor = screens.isEmpty() ? 1.0 : screens.constFirst()->devicePixelRatio();

    width = qRound(rect.width() * scaleFactor);
    height = qRound(rect.height() * scaleFactor);
}

Aspect_FBConfig OcctWindow::NativeFBConfig() const
{
    return nullptr;
}

#ifndef OCCTWINDOW_H
#define OCCTWINDOW_H

#include <Aspect_Window.hxx>

#include <QWidget>

class OcctWindow : public Aspect_Window
{
    DEFINE_STANDARD_RTTIEXT(OcctWindow, Aspect_Window)

public:
    explicit OcctWindow(QWidget *widget,
                        const Quantity_NameOfColor backColor = Quantity_NOC_MATRAGRAY);
    ~OcctWindow() override = default;

    Aspect_Drawable NativeHandle() const override;
    Aspect_Drawable NativeParentHandle() const override;
    Aspect_TypeOfResize DoResize() override;
    Standard_Boolean IsMapped() const override;
    Standard_Boolean DoMapping() const override;
    void Map() const override;
    void Unmap() const override;
    void Position(Standard_Integer &x1,
                  Standard_Integer &y1,
                  Standard_Integer &x2,
                  Standard_Integer &y2) const override;
    Standard_Real Ratio() const override;
    void Size(Standard_Integer &width, Standard_Integer &height) const override;
    Aspect_FBConfig NativeFBConfig() const override;

private:
    QWidget *m_widget;
    Standard_Integer m_xLeft;
    Standard_Integer m_yTop;
    Standard_Integer m_xRight;
    Standard_Integer m_yBottom;
};

#endif // OCCTWINDOW_H

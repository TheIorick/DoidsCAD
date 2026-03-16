#ifndef CADVIEWPORT_H
#define CADVIEWPORT_H

#include <AIS_InteractiveContext.hxx>
#include <AIS_ViewController.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

#include <QWidget>

class CadViewport : public QWidget, protected AIS_ViewController
{
    Q_OBJECT

public:
    explicit CadViewport(QWidget *parent = nullptr);

    QPaintEngine *paintEngine() const override;

public slots:
    void fitAll();
    void setWireframeMode();
    void setShadedMode();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void initializeViewer();
    void displayStartupShape();

    Handle(AIS_InteractiveContext) m_context;
    Handle(V3d_Viewer) m_viewer;
    Handle(V3d_View) m_view;
    Handle(AIS_InteractiveObject) m_startupShape;
};

#endif // CADVIEWPORT_H

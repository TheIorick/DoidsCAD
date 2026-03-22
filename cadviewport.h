#ifndef CADVIEWPORT_H
#define CADVIEWPORT_H

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <AIS_ViewController.hxx>
#include <TopoDS_Shape.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_TypeOfOrientation.hxx>

#include <QWidget>

class CadViewport : public QWidget, protected AIS_ViewController
{
    Q_OBJECT

public:
    explicit CadViewport(QWidget *parent = nullptr);

    QPaintEngine *paintEngine() const override;
    void setShape(const TopoDS_Shape &shape);
    void clearShape();

signals:
    void selectionDescriptionChanged(const QString &description);
    void placementPointPicked(double x, double y, double z);
    void placementCanceled();

public slots:
    void fitAll();
    void setWireframeMode();
    void setShadedMode();
    void setFrontView();
    void setTopView();
    void setRightView();
    void setIsometricView();
    void setDisplayedShapeSelected(bool selected);
    void setHighlightedShape(const TopoDS_Shape &shape);
    void setPlacementPreviewShape(const TopoDS_Shape &shape);
    void startPlacementPick();
    void cancelPlacementPick();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    enum class SurfaceDisplayMode
    {
        Shaded,
        Wireframe
    };

    void initializeViewer();
    void setViewOrientation(V3d_TypeOfOrientation orientation);
    void updateSelectionDescription();
    void clearHighlightPresentation();
    void clearPreviewPresentation();
    void updatePlacementPreview(double x, double y, double z);
    void applyDisplayMode(const Handle(AIS_Shape) &presentation) const;
    void rebuildMainPresentation(bool fitView);
    bool tryPickPlacementPoint(const QPointF &position, double *x, double *y, double *z) const;

    Handle(AIS_InteractiveContext) m_context;
    Handle(V3d_Viewer) m_viewer;
    Handle(V3d_View) m_view;
    Handle(AIS_Shape) m_shapePresentation;
    Handle(AIS_Shape) m_highlightPresentation;
    Handle(AIS_Shape) m_previewPresentation;
    TopoDS_Shape m_currentShape;
    TopoDS_Shape m_previewBaseShape;
    QPointF m_lastPressPosition;
    bool m_isDragging;
    bool m_isPlacementPickMode;
    SurfaceDisplayMode m_surfaceDisplayMode;
};

#endif // CADVIEWPORT_H

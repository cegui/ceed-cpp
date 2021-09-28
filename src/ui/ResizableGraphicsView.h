#ifndef RESIZABLEGRAPHICSVIEW_H
#define RESIZABLEGRAPHICSVIEW_H

#include "qgraphicsview.h"

// If you plan to use ResizableGraphicsRectItems, make sure you view them
// via a GraphicsView that is inherited from this exact class.
// The reason for that is that The ResizableRectItem needs to counter-scale
// resizing handles
// CEGUIGraphicsView inherits from this class because you are likely to use
// resizables on top of CEGUI. If you don't need them, simply don't use them.
// The overhead is minimal.

class QLabel;

class ResizableGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:

    ResizableGraphicsView(QWidget *parent = nullptr);

    void setTransform(const QTransform& transform);
    void zoomIn();
    void zoomOut();
    void zoomReset();

    void setWheelZoomEnabled(bool enabled) { wheelZoomEnabled = enabled; }
    void setHelpEnabled(bool enabled);
    void setMiddleButtonDragScrollEnabled(bool enabled) { middleButtonDragScrollEnabled = enabled; }
    void setHelpText(const QString& text);
    void setHelpVisible(bool show);

    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;

signals:

    void zoomChanged(qreal factor);

protected:

    QLabel* helpLabel = nullptr;

    QPoint lastDragScrollMousePosition;
    qreal zoomFactor = 1.0;
    bool wheelZoomEnabled = false;
    bool _helpEnabled = true;
    bool middleButtonDragScrollEnabled = false;
    bool ctrlZoom = false;
};

#endif // RESIZABLEGRAPHICSVIEW_H

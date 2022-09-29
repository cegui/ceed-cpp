#ifndef CEGUIGRAPHICSVIEW_H
#define CEGUIGRAPHICSVIEW_H

#include "src/ui/ResizableGraphicsView.h"

// This is a final class, not suitable for subclassing. This views given scene using
// QOpenGLWidget. It's designed to work with CEGUIGraphicsScene derived classes.

class QOpenGLTextureBlitter;

class CEGUIGraphicsView final : public ResizableGraphicsView
{
    Q_OBJECT

public:

    explicit CEGUIGraphicsView(QWidget *parent = nullptr);
    virtual ~CEGUIGraphicsView() override;

    void injectInput(bool inject);
    void setContinuousRendering(bool on) { continuousRendering = on; }

    virtual void drawBackground(QPainter* painter, const QRectF& rect) override;

signals:

    void cursorPositionChanged(float x, float y);

private:

    void updateCheckerboardBrush();

    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;

    QOpenGLTextureBlitter* blitter = nullptr;
    QBrush checkerboardBrush;

    bool _injectInput = false;

    // if true, we render always (possibly capped to some FPS) - suitable for live preview
    // if false, we render only when update() is called - suitable for visual editing
    bool continuousRendering = true;
};

#endif // CEGUIGRAPHICSVIEW_H

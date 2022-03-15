#ifndef RESIZABLERECTITEM_H
#define RESIZABLERECTITEM_H

#include "qgraphicsitem.h"

// Rectangle that can be resized by dragging it's handles.
// Inherit from this class to gain resizing and moving capabilities.
// Depending on the size, the handles are shown outside the rectangle (if it's small)
// or inside (if it's large). All this is tweakable.

class ResizingHandle;
class QMouseEvent;

class ResizableRectItem : public QGraphicsRectItem
{
public:

    ResizableRectItem(QGraphicsItem *parent = nullptr);

    virtual bool isAnyHandleSelected() const;
    virtual void deselectAllHandles();
    void hideAllHandles(const QGraphicsItem* excluding = nullptr);
    void setResizingEnabled(bool enabled = true);
    void setRect(QRectF newRect);
    void setRect(qreal ax, qreal ay, qreal w, qreal h) { setRect(QRectF(ax, ay, w, h)); }
    bool moveInProgress() const { return _moveInProgress; }
    bool resizeInProgress() const { return _resizeInProgress; }
    QPointF getResizeStartPos() const { return _resizeStartPos; }
    QRectF getResizeStartRect() const { return _resizeStartRect; }

    void beginResizing(const QGraphicsItem& handle);
    void performResizing(qreal deltaLeft, qreal deltaTop, qreal deltaRight, qreal deltaBottom);
    void endResizing();
    void beginMoving();
    void performMoving(QPointF& delta, bool external);
    void endMoving();

    virtual QPointF constrainMovePoint(QPointF value) { return value; }
    virtual QRectF constrainResizeRect(QRectF rect, QRectF oldRect);
    virtual QSizeF getMinSize() const { return QSizeF(1.0, 1.0); }
    virtual QSizeF getMaxSize() const { return QSizeF(std::numeric_limits<qreal>().max(), std::numeric_limits<qreal>().max()); }

    virtual void onScaleChanged(qreal scaleX, qreal scaleY);
    void mouseReleaseEventSelected();

    virtual void notifyHandleSelected(ResizingHandle* /*handle*/) {}
    virtual void notifyResizeStarted() {}
    virtual void notifyResizeProgress(QPointF /*newPos*/, QSizeF /*newSize*/) {}
    virtual void notifyResizeFinished(QPointF /*newPos*/, QSizeF /*newSize*/) {}
    virtual void notifyMoveStarted() {}
    virtual void notifyMoveProgress(QPointF /*newPos*/) {}
    virtual void notifyMoveFinished(QPointF /*newPos*/) {}

protected:

    virtual void updateHandles();

    virtual QPen getNormalPen() const;
    virtual QPen getHoverPen() const;
    virtual QPen getPenWhileResizing() const;
    virtual QPen getPenWhileMoving() const;

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

    ResizingHandle* topEdgeHandle = nullptr;
    ResizingHandle* bottomEdgeHandle = nullptr;
    ResizingHandle* leftEdgeHandle = nullptr;
    ResizingHandle* rightEdgeHandle = nullptr;
    ResizingHandle* topRightCornerHandle = nullptr;
    ResizingHandle* bottomRightCornerHandle = nullptr;
    ResizingHandle* bottomLeftCornerHandle = nullptr;
    ResizingHandle* topLeftCornerHandle = nullptr;

    qreal _currentScaleX = 1.0;
    qreal _currentScaleY = 1.0;

    int _outerHandleSize = 15;
    int _innerHandleSize = 10;
    bool _handlesDirty = true;

    QPointF _moveStartPos;
    QPointF _resizeStartPos;
    QRectF _resizeStartRect;

    bool _mouseOver = false;
    bool _moveInProgress = false;
    bool _resizeInProgress = false;
    bool _ignoreGeometryChanges = false;
};

#endif // RESIZABLERECTITEM_H

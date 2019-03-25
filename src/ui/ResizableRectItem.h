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

    void unselectAllHandles();
    void hideAllHandles();

    virtual void performResizing(const ResizingHandle& handle, qreal& deltaLeft, qreal& deltaTop, qreal& deltaRight, qreal& deltaBottom);
    virtual QPointF constrainMovePoint(QPointF value) { return value; }
    virtual QRectF constrainResizeRect(QRectF rect, QRectF oldRect);

    void onScaleChanged(qreal scaleX, qreal scaleY);
    void mouseReleaseEventSelected(QMouseEvent* event);

    virtual void notifyHandleSelected(ResizingHandle* handle) = 0;
    virtual void notifyResizeStarted() = 0;
    virtual void notifyResizeProgress(QPointF newPos, QRectF newRect) = 0;
    virtual void notifyResizeFinished(QPointF newPos, QRectF newRect);
    virtual void notifyMoveStarted() = 0;
    virtual void notifyMoveProgress(QPointF newPos) = 0;
    virtual void notifyMoveFinished(QPointF newPos) = 0;

protected:

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

    QPointF moveOldPos;

    qreal _currentScaleX = 1.0;
    qreal _currentScaleY = 1.0;

    bool _handlesDirty = true;
    bool _mouseOver = false;
    bool _moveInProgress = false;
    bool _resizeInProgress = false;
    bool _ignoreGeometryChanges = false;
};

#endif // RESIZABLERECTITEM_H

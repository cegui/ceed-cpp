#include "src/ui/ResizingHandle.h"
#include "src/ui/ResizableRectItem.h"
#include "qcursor.h"

ResizingHandle::ResizingHandle(ResizableRectItem* parent)
    : QGraphicsRectItem(parent)
{
    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);

    if (isEdge())
    {
        //setPen(parent->getEdgeResizingHandleHiddenPen())

        setCursor(isHorizontal() ? Qt::SizeVerCursor : Qt::SizeHorCursor);
    }
    else
    {
        //self.setPen(parent->getCornerResizingHandleHiddenPen())

        setZValue(1.0);

        if (_type == Type::TopRight || _type == Type::BottomLeft)
            setCursor(Qt::SizeBDiagCursor);
        else
            setCursor(Qt::SizeFDiagCursor);
    }

}

// Adjusts the parent rectangle and returns a position to use for this handle (with restrictions accounted for)
QPointF ResizingHandle::performResizing(QPointF value)
{
    auto delta = value - pos();

    qreal left = 0.0;
    qreal top = 0.0;
    qreal right = 0.0;
    qreal bottom = 0.0;

    // Each handle type influences only a subset of params
    switch (_type)
    {
        case Type::Top: top = delta.y(); break;
        case Type::Bottom: bottom = delta.y(); break;
        case Type::Left: left = delta.x(); break;
        case Type::Right: right = delta.x(); break;
        case Type::TopLeft: top = delta.y(); left = delta.x(); break;
        case Type::BottomRight: bottom = delta.y(); right = delta.x(); break;
        case Type::BottomLeft: bottom = delta.y(); left = delta.x(); break;
        case Type::TopRight: top = delta.y(); right = delta.x(); break;
    }

    // Modifies left, right, top & bottom inside, results are actual changes
    ResizableRectItem* parentResizable = static_cast<ResizableRectItem*>(parentItem());
    /*
        parentResizable.performResizing(self, left, top, right, bottom)
    */

    return QPointF(left + right + pos().x(), top + bottom + pos().y());
}

void ResizingHandle::onScaleChanged(qreal scaleX, qreal scaleY)
{
    auto tfm = transform();

    const qreal counterScaleX = isHorizontal() ? 1.0 : (1.0 / scaleX);
    const qreal counterScaleY = isVertical() ? 1.0 : (1.0 / scaleY);

    tfm = QTransform(counterScaleX, tfm.m12(), tfm.m13(),
                     tfm.m21(), counterScaleY, tfm.m23(),
                     tfm.m31(), tfm.m32(), tfm.m33());

    setTransform(tfm);
}

// Called when mouse is released whilst this was selected. This notifies us that resizing might have ended.
void ResizingHandle::mouseReleaseEventSelected(QMouseEvent* event)
{
    ResizableRectItem* parentResizable = static_cast<ResizableRectItem*>(parentItem());
/*
    if parentResizable->resizeInProgress:
        # resize was in progress and just ended
        parentResizable->resizeInProgress = False
        parentResizable->setPen(parentResizable->getHoverPen() if parentResizable->mouseOver else parentResizable->getNormalPen())

        newPos = parentResizable->pos() + parentResizable->rect().topLeft()
        newRect = QtCore.QRectF(0, 0, parentResizable->rect().width(), parentResizable->rect().height())

        parentResizable->notifyResizeFinished(newPos, newRect)
*/
}

// This method does most of the resize work
QVariant ResizingHandle::itemChange(GraphicsItemChange change, const QVariant& value)
{
    ResizableRectItem* parentResizable = static_cast<ResizableRectItem*>(parentItem());
/*
        if change == QtGui.QGraphicsItem.ItemSelectedChange:
            if parentResizable->isSelected():
                # we disallow multi-selecting a resizable item and one of it's handles,
                return False

        elif change == QtGui.QGraphicsItem.ItemSelectedHasChanged:
            # if we have indeed been selected, make sure all our sibling handles are unselected
            # we allow multi-selecting multiple handles but only one handle per resizable is allowed

            // Make sure all siblings of this handle are unselected
            for item in parentResizable->childItems():
                if isinstance(item, ResizingHandle) and not self is item:
                    item.setSelected(False)

            parentResizable->notifyHandleSelected(self)

        elif change == QtGui.QGraphicsItem.ItemPositionChange:
            # this is the money code
            # changing position of the handle resizes the whole resizable
            if not parentResizable->resizeInProgress and not _ignoreGeometryChanges:
                parentResizable->resizeInProgress = True
                parentResizable->resizeOldPos = parentResizable->pos()
                parentResizable->resizeOldRect = parentResizable->rect()

                parentResizable->setPen(parentResizable->getPenWhileResizing())
                parentResizable->hideAllHandles(excluding = self)

                parentResizable->notifyResizeStarted()

            if parentResizable->resizeInProgress:
                ret = self.performResizing(value)

                newPos = parentResizable->pos() + parentResizable->rect().topLeft()
                newRect = QtCore.QRectF(0, 0, parentResizable->rect().width(), parentResizable->rect().height())

                parentResizable->notifyResizeProgress(newPos, newRect)

                return ret

*/
    return QGraphicsRectItem::itemChange(change, value);
}

void ResizingHandle::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    QGraphicsRectItem::hoverEnterEvent(event);
    _mouseOver = true;

    ResizableRectItem* parentResizable = static_cast<ResizableRectItem*>(parentItem());
    if (isEdge())
        ;//setPen(parentResizable->getEdgeResizingHandleHoverPen())
    else
        ;//setPen(parentResizable->getCornerResizingHandleHoverPen())
}

void ResizingHandle::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    ResizableRectItem* parentResizable = static_cast<ResizableRectItem*>(parentItem());
    if (isEdge())
        ;//setPen(parentResizable->getEdgeResizingHandleHiddenPen())
    else
        ;//setPen(parentResizable->getCornerResizingHandleHiddenPen())

    _mouseOver = false;
    QGraphicsRectItem::hoverLeaveEvent(event);
}

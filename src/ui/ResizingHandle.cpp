#include "src/ui/ResizingHandle.h"
#include "qcursor.h"

ResizingHandle::ResizingHandle(QGraphicsItem* parent)
    : QGraphicsRectItem(parent)
{
    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);

/*
        self.parentResizable = parent

        self.ignoreGeometryChanges = False
        self.ignoreTransformChanges = False
        self.currentView = None
*/

    if (isEdge())
    {
        //setPen(self.parentResizable.getEdgeResizingHandleHiddenPen())

        setCursor(isHorizontal() ? Qt::SizeVerCursor : Qt::SizeHorCursor);
    }
    else
    {
        //self.setPen(self.parentResizable.getCornerResizingHandleHiddenPen())

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
/*
        // TOP:

        _, dy1, _, _ = self.parentResizable.performResizing(self, 0, deltaY, 0, 0)

        return QtCore.QPointF(self.pos().x(), dy1 + self.pos().y())

        // BOTTOM:

        _, _, _, dy2 = self.parentResizable.performResizing(self, 0, 0, 0, deltaY)

        return QtCore.QPointF(self.pos().x(), dy2 + self.pos().y())

        // LEFT:

        dx1, _, _, _ = self.parentResizable.performResizing(self, deltaX, 0, 0, 0)

        return QtCore.QPointF(dx1 + self.pos().x(), self.pos().y())

        // RIGHT:

        _, _, dx2, _ = self.parentResizable.performResizing(self, 0, 0, deltaX, 0)

        return QtCore.QPointF(dx2 + self.pos().x(), self.pos().y())

        // TR:

        _, dy1, dx2, _ = self.parentResizable.performResizing(self, 0, deltaY, deltaX, 0)

        return QtCore.QPointF(dx2 + self.pos().x(), dy1 + self.pos().y())

        // BR:

        _, _, dx2, dy2 = self.parentResizable.performResizing(self, 0, 0, deltaX, deltaY)

        return QtCore.QPointF(dx2 + self.pos().x(), dy2 + self.pos().y())

        // BL:

        dx1, _, _, dy2 = self.parentResizable.performResizing(self, deltaX, 0, 0, deltaY)

        return QtCore.QPointF(dx1 + self.pos().x(), dy2 + self.pos().y())

        // TL:

        dx1, dy1, _, _ = self.parentResizable.performResizing(self, deltaX, deltaY, 0, 0)

        return QtCore.QPointF(dx1 + self.pos().x(), dy1 + self.pos().y())
*/
    return QPointF(0.0 + pos().x(), 0.0 + pos().y());
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
/*
    if self.parentResizable.resizeInProgress:
        # resize was in progress and just ended
        self.parentResizable.resizeInProgress = False
        self.parentResizable.setPen(self.parentResizable.getHoverPen() if self.parentResizable.mouseOver else self.parentResizable.getNormalPen())

        newPos = self.parentResizable.pos() + self.parentResizable.rect().topLeft()
        newRect = QtCore.QRectF(0, 0, self.parentResizable.rect().width(), self.parentResizable.rect().height())

        self.parentResizable.notifyResizeFinished(newPos, newRect)
*/
}

// This method does most of the resize work
QVariant ResizingHandle::itemChange(GraphicsItemChange change, const QVariant& value)
{
/*
        if change == QtGui.QGraphicsItem.ItemSelectedChange:
            if self.parentResizable.isSelected():
                # we disallow multi-selecting a resizable item and one of it's handles,
                return False

        elif change == QtGui.QGraphicsItem.ItemSelectedHasChanged:
            # if we have indeed been selected, make sure all our sibling handles are unselected
            # we allow multi-selecting multiple handles but only one handle per resizable is allowed

            // Make sure all siblings of this handle are unselected
            for item in self.parentResizable.childItems():
                if isinstance(item, ResizingHandle) and not self is item:
                    item.setSelected(False)

            self.parentResizable.notifyHandleSelected(self)

        elif change == QtGui.QGraphicsItem.ItemPositionChange:
            # this is the money code
            # changing position of the handle resizes the whole resizable
            if not self.parentResizable.resizeInProgress and not self.ignoreGeometryChanges:
                self.parentResizable.resizeInProgress = True
                self.parentResizable.resizeOldPos = self.parentResizable.pos()
                self.parentResizable.resizeOldRect = self.parentResizable.rect()

                self.parentResizable.setPen(self.parentResizable.getPenWhileResizing())
                self.parentResizable.hideAllHandles(excluding = self)

                self.parentResizable.notifyResizeStarted()

            if self.parentResizable.resizeInProgress:
                ret = self.performResizing(value)

                newPos = self.parentResizable.pos() + self.parentResizable.rect().topLeft()
                newRect = QtCore.QRectF(0, 0, self.parentResizable.rect().width(), self.parentResizable.rect().height())

                self.parentResizable.notifyResizeProgress(newPos, newRect)

                return ret

*/
    return QGraphicsRectItem::itemChange(change, value);
}

void ResizingHandle::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    QGraphicsRectItem::hoverEnterEvent(event);
    mouseOver = true;

    if (isEdge())
        ;//setPen(self.parentResizable.getEdgeResizingHandleHoverPen())
    else
        ;//setPen(self.parentResizable.getCornerResizingHandleHoverPen())
}

void ResizingHandle::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    if (isEdge())
        ;//setPen(self.parentResizable.getEdgeResizingHandleHiddenPen())
    else
        ;//setPen(self.parentResizable.getCornerResizingHandleHiddenPen())

    mouseOver = false;
    QGraphicsRectItem::hoverLeaveEvent(event);
}

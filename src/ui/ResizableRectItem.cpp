#include "src/ui/ResizableRectItem.h"
#include "src/ui/ResizingHandle.h"
#include "qcursor.h"

ResizableRectItem::ResizableRectItem(QGraphicsItem* parent)
    : QGraphicsRectItem(parent)
{
    setFlags(ItemSendsGeometryChanges | ItemIsMovable);
    setAcceptHoverEvents(true);

/*
        self.topEdgeHandle = TopEdgeResizingHandle(self)
        self.bottomEdgeHandle = BottomEdgeResizingHandle(self)
        self.leftEdgeHandle = LeftEdgeResizingHandle(self)
        self.rightEdgeHandle = RightEdgeResizingHandle(self)

        self.topRightCornerHandle = TopRightCornerResizingHandle(self)
        self.bottomRightCornerHandle = BottomRightCornerResizingHandle(self)
        self.bottomLeftCornerHandle = BottomLeftCornerResizingHandle(self)
        self.topLeftCornerHandle = TopLeftCornerResizingHandle(self)

        self.resizeOldPos = None
        self.resizeOldRect = None
        self.moveOldPos = None
*/
    hideAllHandles();
/*
        self.outerHandleSize = 0
        self.innerHandleSize = 0
        self.setOuterHandleSize(15)
        self.setInnerHandleSize(10)

        self.setPen(self.getNormalPen())
*/
    setCursor(Qt::OpenHandCursor);
}

void ResizableRectItem::unselectAllHandles()
{

}

void ResizableRectItem::hideAllHandles()
{

}

// Adjusts the rectangle and returns a 4-tuple of the actual used deltas (with restrictions accounted for)
// Deltas are in-out, returning the actual change applied. The default implementation doesn't use the handle parameter.
void ResizableRectItem::performResizing(const ResizingHandle& handle, qreal& deltaLeft, qreal& deltaTop, qreal& deltaRight, qreal& deltaBottom)
{
    auto newRect = rect().adjusted(deltaLeft, deltaTop, deltaRight, deltaBottom);
    newRect = constrainResizeRect(newRect, rect());

    // TODO: the rect moves as a whole when it can't be sized any less
    //       this is probably not the behavior we want!

    deltaLeft = newRect.left() - rect().left();
    deltaTop = newRect.top() - rect().top();
    deltaRight = newRect.right() - rect().right();
    deltaBottom = newRect.bottom() - rect().bottom();

    setRect(newRect);
}

QRectF ResizableRectItem::constrainResizeRect(QRectF rect, QRectF oldRect)
{
/*
        minSize = self.getMinSize()
        maxSize = self.getMaxSize()

        if minSize:
            minRect = QtCore.QRectF(rect.center() - QtCore.QPointF(0.5 * minSize.width(), 0.5 * minSize.height()), minSize)
            rect = rect.united(minRect)
        if maxSize:
            maxRect = QtCore.QRectF(rect.center() - QtCore.QPointF(0.5 * maxSize.width(), 0.5 * maxSize.height()), maxSize)
            rect.intersected(maxRect)
*/
    return rect;
}

void ResizableRectItem::onScaleChanged(qreal scaleX, qreal scaleY)
{
    _currentScaleX = scaleX;
    _currentScaleY = scaleY;

    for (QGraphicsItem* item : childItems())
    {
        ResizingHandle* handle = dynamic_cast<ResizingHandle*>(item);
        if (handle)
        {
            handle->onScaleChanged(scaleX, scaleY);
            continue;
        }

        ResizableRectItem* rectItem = dynamic_cast<ResizableRectItem*>(item);
        if (rectItem)
        {
            rectItem->onScaleChanged(scaleX, scaleY);
            continue;
        }
    }

    _handlesDirty = true;

/*
        self.ensureHandlesUpdated()
*/
}

void ResizableRectItem::mouseReleaseEventSelected(QMouseEvent* event)
{
    if (_moveInProgress)
    {
        _moveInProgress = false;
        notifyMoveFinished(pos());
    }
}

void ResizableRectItem::notifyResizeFinished(QPointF newPos, QRectF newRect)
{
    _ignoreGeometryChanges = true;
    setRect(newRect);
    setPos(newPos);
    _ignoreGeometryChanges = false;
}

QVariant ResizableRectItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemSelectedHasChanged)
    {
        if (value.toBool())
            unselectAllHandles();
        else
            hideAllHandles();
    }
    else if (change == ItemPositionChange)
    {
        auto point = constrainMovePoint(value.toPointF());
        if (!_moveInProgress && !_ignoreGeometryChanges)
        {
            _moveInProgress = true;
            moveOldPos = pos();

            /*
                setPen(self.getPenWhileMoving())
            */

            hideAllHandles();

            notifyMoveStarted();
        }

        if (_moveInProgress)
        {
            // 'point' is the new position, self.pos() is the old position,
            // we use 'point' to avoid the 1 pixel lag
            notifyMoveProgress(point);
        }
    }

    return QGraphicsRectItem::itemChange(change, value);
}

void ResizableRectItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    QGraphicsRectItem::hoverEnterEvent(event);
    /*
        setPen(self.getHoverPen())
    */
    _mouseOver = true;
}

void ResizableRectItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    _mouseOver = false;
    /*
        setPen(self.getNormalPen())
    */
    QGraphicsRectItem::hoverLeaveEvent(event);
}

/*
    def getMinSize(self):
        ret = QtCore.QSizeF(1, 1)

        return ret

    def getMaxSize(self):
        return None

    def getNormalPen(self):
        ret = QtGui.QPen()
        ret.setColor(QtGui.QColor(255, 255, 255, 150))
        ret.setStyle(QtCore.Qt.DotLine)

        return ret

    def getHoverPen(self):
        ret = QtGui.QPen()
        ret.setColor(QtGui.QColor(0, 255, 255, 255))

        return ret

    def getPenWhileResizing(self):
        ret = QtGui.QPen(QtGui.QColor(255, 0, 255, 255))

        return ret

    def getPenWhileMoving(self):
        ret = QtGui.QPen(QtGui.QColor(255, 0, 255, 255))

        return ret

    def setOuterHandleSize(self, size):
        self.outerHandleSize = size
        self.handlesDirty = True

    def setInnerHandleSize(self, size):
        self.innerHandleSize = size
        self.handlesDirty = True

    def setRect(self, rect):
        super(ResizableRectItem, self).setRect(rect)

        self.handlesDirty = True
        self.ensureHandlesUpdated()

    def hideAllHandles(self, excluding = None):
        """Hides all handles. If a handle is given as the 'excluding' parameter, this handle is
        skipped over when hiding
        """

        for item in self.childItems():
            if isinstance(item, ResizingHandle) and item is not excluding:
                if isinstance(item, EdgeResizingHandle):
                    item.setPen(self.getEdgeResizingHandleHiddenPen())

                elif isinstance(item, CornerResizingHandle):
                    item.setPen(self.getCornerResizingHandleHiddenPen())

    def setResizingEnabled(self, enabled = True):
        """Makes it possible to disable or enable resizing
        """

        for item in self.childItems():
            if isinstance(item, ResizingHandle):
                item.setVisible(enabled)

    def unselectAllHandles(self):
        """Unselects all handles of this resizable"""

        for item in self.childItems():
            if isinstance(item, ResizingHandle):
                item.setSelected(False)

    def isAnyHandleSelected(self):
        """Checks whether any of the 8 handles is selected.
        note: At most 1 handle can be selected at a time!"""
        for item in self.childItems():
            if isinstance(item, ResizingHandle):
                if item.isSelected():
                    return True

        return False

    def ensureHandlesUpdated(self):
        """Makes sure handles are updated (if possible).
        Updating handles while resizing would mess things up big times, so we just ignore the
        update in that circumstance
        """

        if self.handlesDirty and not self.resizeInProgress:
            self.updateHandles()

    def absoluteXToRelative(self, value, transform):
        xScale = transform.m11()

        # this works in this special case, not in generic case!
        # I would have to undo rotation for this to work generically
        return value / xScale if xScale != 0 else 1

    def absoluteYToRelative(self, value, transform):
        yScale = transform.m22()

        # this works in this special case, not in generic case!
        # I would have to undo rotation for this to work generically
        return value / yScale if yScale != 0 else 1

    def updateHandles(self):
        """Updates all the handles according to geometry"""

        absoluteWidth = self.currentScaleX * self.rect().width()
        absoluteHeight = self.currentScaleY * self.rect().height()

        if absoluteWidth < 4 * self.outerHandleSize or absoluteHeight < 4 * self.outerHandleSize:
            self.topEdgeHandle.ignoreGeometryChanges = True
            self.topEdgeHandle.setPos(0, 0)
            self.topEdgeHandle.setRect(0, -self.innerHandleSize,
                                       self.rect().width(),
                                       self.innerHandleSize)
            self.topEdgeHandle.ignoreGeometryChanges = False

            self.bottomEdgeHandle.ignoreGeometryChanges = True
            self.bottomEdgeHandle.setPos(0, self.rect().height())
            self.bottomEdgeHandle.setRect(0, 0,
                                       self.rect().width(),
                                       self.innerHandleSize)
            self.bottomEdgeHandle.ignoreGeometryChanges = False

            self.leftEdgeHandle.ignoreGeometryChanges = True
            self.leftEdgeHandle.setPos(0, 0)
            self.leftEdgeHandle.setRect(-self.innerHandleSize, 0,
                                       self.innerHandleSize,
                                       self.rect().height())
            self.leftEdgeHandle.ignoreGeometryChanges = False

            self.rightEdgeHandle.ignoreGeometryChanges = True
            self.rightEdgeHandle.setPos(QtCore.QPointF(self.rect().width(), 0))
            self.rightEdgeHandle.setRect(0, 0,
                                       self.innerHandleSize,
                                       self.rect().height())
            self.rightEdgeHandle.ignoreGeometryChanges = False

            self.topRightCornerHandle.ignoreGeometryChanges = True
            self.topRightCornerHandle.setPos(self.rect().width(), 0)
            self.topRightCornerHandle.setRect(0, -self.innerHandleSize,
                                       self.innerHandleSize,
                                       self.innerHandleSize)
            self.topRightCornerHandle.ignoreGeometryChanges = False

            self.bottomRightCornerHandle.ignoreGeometryChanges = True
            self.bottomRightCornerHandle.setPos(self.rect().width(), self.rect().height())
            self.bottomRightCornerHandle.setRect(0, 0,
                                       self.innerHandleSize,
                                       self.innerHandleSize)
            self.bottomRightCornerHandle.ignoreGeometryChanges = False

            self.bottomLeftCornerHandle.ignoreGeometryChanges = True
            self.bottomLeftCornerHandle.setPos(0, self.rect().height())
            self.bottomLeftCornerHandle.setRect(-self.innerHandleSize, 0,
                                       self.innerHandleSize,
                                       self.innerHandleSize)
            self.bottomLeftCornerHandle.ignoreGeometryChanges = False

            self.topLeftCornerHandle.ignoreGeometryChanges = True
            self.topLeftCornerHandle.setPos(0, 0)
            self.topLeftCornerHandle.setRect(-self.innerHandleSize, -self.innerHandleSize,
                                       self.innerHandleSize,
                                       self.innerHandleSize)
            self.topLeftCornerHandle.ignoreGeometryChanges = False

        else:
            self.topEdgeHandle.ignoreGeometryChanges = True
            self.topEdgeHandle.setPos(0, 0)
            self.topEdgeHandle.setRect(0, 0,
                                       self.rect().width(),
                                       self.outerHandleSize)
            self.topEdgeHandle.ignoreGeometryChanges = False

            self.bottomEdgeHandle.ignoreGeometryChanges = True
            self.bottomEdgeHandle.setPos(0, self.rect().height())
            self.bottomEdgeHandle.setRect(0, -self.outerHandleSize,
                                       self.rect().width(),
                                       self.outerHandleSize)
            self.bottomEdgeHandle.ignoreGeometryChanges = False

            self.leftEdgeHandle.ignoreGeometryChanges = True
            self.leftEdgeHandle.setPos(QtCore.QPointF(0, 0))
            self.leftEdgeHandle.setRect(0, 0,
                                       self.outerHandleSize,
                                       self.rect().height())
            self.leftEdgeHandle.ignoreGeometryChanges = False

            self.rightEdgeHandle.ignoreGeometryChanges = True
            self.rightEdgeHandle.setPos(QtCore.QPointF(self.rect().width(), 0))
            self.rightEdgeHandle.setRect(-self.outerHandleSize, 0,
                                       self.outerHandleSize,
                                       self.rect().height())
            self.rightEdgeHandle.ignoreGeometryChanges = False

            self.topRightCornerHandle.ignoreGeometryChanges = True
            self.topRightCornerHandle.setPos(self.rect().width(), 0)
            self.topRightCornerHandle.setRect(-self.outerHandleSize, 0,
                                       self.outerHandleSize,
                                       self.outerHandleSize)
            self.topRightCornerHandle.ignoreGeometryChanges = False

            self.bottomRightCornerHandle.ignoreGeometryChanges = True
            self.bottomRightCornerHandle.setPos(self.rect().width(), self.rect().height())
            self.bottomRightCornerHandle.setRect(-self.outerHandleSize, -self.outerHandleSize,
                                       self.outerHandleSize,
                                       self.outerHandleSize)
            self.bottomRightCornerHandle.ignoreGeometryChanges = False

            self.bottomLeftCornerHandle.ignoreGeometryChanges = True
            self.bottomLeftCornerHandle.setPos(0, self.rect().height())
            self.bottomLeftCornerHandle.setRect(0, -self.outerHandleSize,
                                       self.outerHandleSize,
                                       self.outerHandleSize)
            self.bottomLeftCornerHandle.ignoreGeometryChanges = False

            self.topLeftCornerHandle.ignoreGeometryChanges = True
            self.topLeftCornerHandle.setPos(0, 0)
            self.topLeftCornerHandle.setRect(0, 0,
                                       self.outerHandleSize,
                                       self.outerHandleSize)
            self.topLeftCornerHandle.ignoreGeometryChanges = False

        self.handlesDirty = False
*/

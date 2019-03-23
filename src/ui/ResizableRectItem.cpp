#include "src/ui/ResizableRectItem.h"

ResizableRectItem::ResizableRectItem(QGraphicsItem* parent)
    : QGraphicsRectItem(parent)
{
    setFlags(ItemSendsGeometryChanges | ItemIsMovable);
    setAcceptHoverEvents(true);

/*
        self.mouseOver = False

        self.topEdgeHandle = TopEdgeResizingHandle(self)
        self.bottomEdgeHandle = BottomEdgeResizingHandle(self)
        self.leftEdgeHandle = LeftEdgeResizingHandle(self)
        self.rightEdgeHandle = RightEdgeResizingHandle(self)

        self.topRightCornerHandle = TopRightCornerResizingHandle(self)
        self.bottomRightCornerHandle = BottomRightCornerResizingHandle(self)
        self.bottomLeftCornerHandle = BottomLeftCornerResizingHandle(self)
        self.topLeftCornerHandle = TopLeftCornerResizingHandle(self)

        self.handlesDirty = True
        self.currentScaleX = 1
        self.currentScaleY = 1

        self.ignoreGeometryChanges = False

        self.resizeInProgress = False
        self.resizeOldPos = None
        self.resizeOldRect = None
        self.moveInProgress = False
        self.moveOldPos = None

        self.hideAllHandles()

        self.outerHandleSize = 0
        self.innerHandleSize = 0
        self.setOuterHandleSize(15)
        self.setInnerHandleSize(10)

        self.setCursor(QtCore.Qt.OpenHandCursor)
        self.setPen(self.getNormalPen())
*/
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

    def getEdgeResizingHandleHoverPen(self):
        ret = QtGui.QPen()
        ret.setColor(QtGui.QColor(0, 255, 255, 255))
        ret.setWidth(2)
        ret.setCosmetic(True)

        return ret

    def getEdgeResizingHandleHiddenPen(self):
        ret = QtGui.QPen()
        ret.setColor(QtCore.Qt.transparent)

        return ret

    def getCornerResizingHandleHoverPen(self):
        ret = QtGui.QPen()
        ret.setColor(QtGui.QColor(0, 255, 255, 255))
        ret.setWidth(2)
        ret.setCosmetic(True)

        return ret

    def getCornerResizingHandleHiddenPen(self):
        ret = QtGui.QPen()
        ret.setColor(QtCore.Qt.transparent)

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

    def constrainMovePoint(self, point):
        return point

    def constrainResizeRect(self, rect, oldRect):
        minSize = self.getMinSize()
        maxSize = self.getMaxSize()

        if minSize:
            minRect = QtCore.QRectF(rect.center() - QtCore.QPointF(0.5 * minSize.width(), 0.5 * minSize.height()), minSize)
            rect = rect.united(minRect)
        if maxSize:
            maxRect = QtCore.QRectF(rect.center() - QtCore.QPointF(0.5 * maxSize.width(), 0.5 * maxSize.height()), maxSize)
            rect.intersected(maxRect)

        return rect

    def performResizing(self, handle, deltaX1, deltaY1, deltaX2, deltaY2):
        """Adjusts the rectangle and returns a 4-tuple of the actual used deltas
        (with restrictions accounted for)

        The default implementation doesn't use the handle parameter.
        """

        newRect = self.rect().adjusted(deltaX1, deltaY1, deltaX2, deltaY2)
        newRect = self.constrainResizeRect(newRect, self.rect())

        # TODO: the rect moves as a whole when it can't be sized any less
        #       this is probably not the behavior we want!

        topLeftDelta = newRect.topLeft() - self.rect().topLeft()
        bottomRightDelta = newRect.bottomRight() - self.rect().bottomRight()

        self.setRect(newRect)

        return topLeftDelta.x(), topLeftDelta.y(), bottomRightDelta.x(), bottomRightDelta.y()

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

    def notifyHandleSelected(self, handle):
        """A method meant to be overridden when you want to react when a handle is selected
        """

        pass

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

    def notifyResizeStarted(self):
        pass

    def notifyResizeProgress(self, newPos, newRect):
        pass

    def notifyResizeFinished(self, newPos, newRect):
        self.ignoreGeometryChanges = True
        self.setRect(newRect)
        self.setPos(newPos)
        self.ignoreGeometryChanges = False

    def notifyMoveStarted(self):
        pass

    def notifyMoveProgress(self, newPos):
        pass

    def notifyMoveFinished(self, newPos):
        pass

    def scaleChanged(self, sx, sy):
        self.currentScaleX = sx
        self.currentScaleY = sy

        for childItem in self.childItems():
            if isinstance(childItem, ResizingHandle):
                childItem.scaleChanged(sx, sy)

            elif isinstance(childItem, ResizableRectItem):
                childItem.scaleChanged(sx, sy)

        self.handlesDirty = True
        self.ensureHandlesUpdated()

    def itemChange(self, change, value):
        if change == QtGui.QGraphicsItem.ItemSelectedHasChanged:
            if value:
                self.unselectAllHandles()
            else:
                self.hideAllHandles()

        elif change == QtGui.QGraphicsItem.ItemPositionChange:
            value = self.constrainMovePoint(value)

            if not self.moveInProgress and not self.ignoreGeometryChanges:
                self.moveInProgress = True
                self.moveOldPos = self.pos()

                self.setPen(self.getPenWhileMoving())
                self.hideAllHandles()

                self.notifyMoveStarted()

            if self.moveInProgress:
                # value is the new position, self.pos() is the old position
                # we use value to avoid the 1 pixel lag
                self.notifyMoveProgress(value)

        return super(ResizableRectItem, self).itemChange(change, value)

    def hoverEnterEvent(self, event):
        super(ResizableRectItem, self).hoverEnterEvent(event)

        self.setPen(self.getHoverPen())
        self.mouseOver = True

    def hoverLeaveEvent(self, event):
        self.mouseOver = False
        self.setPen(self.getNormalPen())

        super(ResizableRectItem, self).hoverLeaveEvent(event)

    def mouseReleaseEventSelected(self, event):
        if self.moveInProgress:
            self.moveInProgress = False
            newPos = self.pos()

            self.notifyMoveFinished(newPos)
*/

#include "src/ui/ResizingHandle.h"

ResizingHandle::ResizingHandle(QGraphicsItem* parent)
    : QGraphicsRectItem(parent)
{
    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);

/*
        self.parentResizable = parent

        self.ignoreGeometryChanges = False
        self.ignoreTransformChanges = False
        self.mouseOver = False
        self.currentView = None
*/
}

/*
    def performResizing(self, value):
        """Adjusts the parent rectangle and returns a position to use for this handle
        (with restrictions accounted for)
        """

        # designed to be overriden
        # pylint: disable-msg=R0201

        return value

    def unselectAllSiblingHandles(self):
        """Makes sure all siblings of this handle are unselected."""

        assert(self.parentResizable)

        for item in self.parentResizable.childItems():
            if isinstance(item, ResizingHandle) and not self is item:
                item.setSelected(False)

    def itemChange(self, change, value):
        """This overriden method does most of the resize work
        """

        if change == QtGui.QGraphicsItem.ItemSelectedChange:
            if self.parentResizable.isSelected():
                # we disallow multi-selecting a resizable item and one of it's handles,
                return False

        elif change == QtGui.QGraphicsItem.ItemSelectedHasChanged:
            # if we have indeed been selected, make sure all our sibling handles are unselected
            # we allow multi-selecting multiple handles but only one handle per resizable is allowed

            self.unselectAllSiblingHandles()
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

        return super(ResizingHandle, self).itemChange(change, value)

    def mouseReleaseEventSelected(self, event):
        """Called when mouse is released whilst this was selected.
        This notifies us that resizing might have ended.
        """

        if self.parentResizable.resizeInProgress:
            # resize was in progress and just ended
            self.parentResizable.resizeInProgress = False
            self.parentResizable.setPen(self.parentResizable.getHoverPen() if self.parentResizable.mouseOver else self.parentResizable.getNormalPen())

            newPos = self.parentResizable.pos() + self.parentResizable.rect().topLeft()
            newRect = QtCore.QRectF(0, 0, self.parentResizable.rect().width(), self.parentResizable.rect().height())

            self.parentResizable.notifyResizeFinished(newPos, newRect)

    def hoverEnterEvent(self, event):
        super(ResizingHandle, self).hoverEnterEvent(event)

        self.mouseOver = True

    def hoverLeaveEvent(self, event):
        self.mouseOver = False

        super(ResizingHandle, self).hoverLeaveEvent(event)

    def scaleChanged(self, sx, sy):
        pass

class EdgeResizingHandle(ResizingHandle):
    """Resizing handle positioned on one of the 4 edges
    """

    def __init__(self, parent):
        super(EdgeResizingHandle, self).__init__(parent)

        self.setPen(self.parentResizable.getEdgeResizingHandleHiddenPen())

    def hoverEnterEvent(self, event):
        super(EdgeResizingHandle, self).hoverEnterEvent(event)

        self.setPen(self.parentResizable.getEdgeResizingHandleHoverPen())

    def hoverLeaveEvent(self, event):
        self.setPen(self.parentResizable.getEdgeResizingHandleHiddenPen())

        super(EdgeResizingHandle, self).hoverLeaveEvent(event)

class TopEdgeResizingHandle(EdgeResizingHandle):
    def __init__(self, parent):
        super(TopEdgeResizingHandle, self).__init__(parent)

        self.setCursor(QtCore.Qt.SizeVerCursor)

    def performResizing(self, value):
        delta = value.y() - self.pos().y()
        _, dy1, _, _ = self.parentResizable.performResizing(self, 0, delta, 0, 0)

        return QtCore.QPointF(self.pos().x(), dy1 + self.pos().y())

    def scaleChanged(self, sx, sy):
        super(TopEdgeResizingHandle, self).scaleChanged(sx, sy)

        transform = self.transform()
        transform = QtGui.QTransform(1.0, transform.m12(), transform.m13(),
                                     transform.m21(), 1.0 / sy, transform.m23(),
                                     transform.m31(), transform.m32(), transform.m33())
        self.setTransform(transform)

class BottomEdgeResizingHandle(EdgeResizingHandle):
    def __init__(self, parent):
        super(BottomEdgeResizingHandle, self).__init__(parent)

        self.setCursor(QtCore.Qt.SizeVerCursor)

    def performResizing(self, value):
        delta = value.y() - self.pos().y()
        _, _, _, dy2 = self.parentResizable.performResizing(self, 0, 0, 0, delta)

        return QtCore.QPointF(self.pos().x(), dy2 + self.pos().y())

    def scaleChanged(self, sx, sy):
        super(BottomEdgeResizingHandle, self).scaleChanged(sx, sy)

        transform = self.transform()
        transform = QtGui.QTransform(1.0, transform.m12(), transform.m13(),
                                     transform.m21(), 1.0 / sy, transform.m23(),
                                     transform.m31(), transform.m32(), transform.m33())
        self.setTransform(transform)

class LeftEdgeResizingHandle(EdgeResizingHandle):
    def __init__(self, parent):
        super(LeftEdgeResizingHandle, self).__init__(parent)

        self.setCursor(QtCore.Qt.SizeHorCursor)

    def performResizing(self, value):
        delta = value.x() - self.pos().x()
        dx1, _, _, _ = self.parentResizable.performResizing(self, delta, 0, 0, 0)

        return QtCore.QPointF(dx1 + self.pos().x(), self.pos().y())

    def scaleChanged(self, sx, sy):
        super(LeftEdgeResizingHandle, self).scaleChanged(sx, sy)

        transform = self.transform()
        transform = QtGui.QTransform(1.0 / sx, transform.m12(), transform.m13(),
                                     transform.m21(), 1.0, transform.m23(),
                                     transform.m31(), transform.m32(), transform.m33())
        self.setTransform(transform)

class RightEdgeResizingHandle(EdgeResizingHandle):
    def __init__(self, parent):
        super(RightEdgeResizingHandle, self).__init__(parent)

        self.setCursor(QtCore.Qt.SizeHorCursor)

    def performResizing(self, value):
        delta = value.x() - self.pos().x()
        _, _, dx2, _ = self.parentResizable.performResizing(self, 0, 0, delta, 0)

        return QtCore.QPointF(dx2 + self.pos().x(), self.pos().y())

    def scaleChanged(self, sx, sy):
        super(RightEdgeResizingHandle, self).scaleChanged(sx, sy)

        transform = self.transform()
        transform = QtGui.QTransform(1.0 / sx, transform.m12(), transform.m13(),
                                     transform.m21(), 1.0, transform.m23(),
                                     transform.m31(), transform.m32(), transform.m33())
        self.setTransform(transform)

class CornerResizingHandle(ResizingHandle):
    """Resizing handle positioned in one of the 4 corners.
    """

    def __init__(self, parent):
        super(CornerResizingHandle, self).__init__(parent)

        self.setPen(self.parentResizable.getCornerResizingHandleHiddenPen())
        self.setFlags(self.flags())

        self.setZValue(1)

    def scaleChanged(self, sx, sy):
        super(CornerResizingHandle, self).scaleChanged(sx, sy)

        transform = self.transform()
        transform = QtGui.QTransform(1.0 / sx, transform.m12(), transform.m13(),
                                     transform.m21(), 1.0 / sy, transform.m23(),
                                     transform.m31(), transform.m32(), transform.m33())
        self.setTransform(transform)

    def hoverEnterEvent(self, event):
        super(CornerResizingHandle, self).hoverEnterEvent(event)

        self.setPen(self.parentResizable.getCornerResizingHandleHoverPen())

    def hoverLeaveEvent(self, event):
        self.setPen(self.parentResizable.getCornerResizingHandleHiddenPen())

        super(CornerResizingHandle, self).hoverLeaveEvent(event)

class TopRightCornerResizingHandle(CornerResizingHandle):
    def __init__(self, parent):
        super(TopRightCornerResizingHandle, self).__init__(parent)

        self.setCursor(QtCore.Qt.SizeBDiagCursor)

    def performResizing(self, value):
        deltaX = value.x() - self.pos().x()
        deltaY = value.y() - self.pos().y()
        _, dy1, dx2, _ = self.parentResizable.performResizing(self, 0, deltaY, deltaX, 0)

        return QtCore.QPointF(dx2 + self.pos().x(), dy1 + self.pos().y())

class BottomRightCornerResizingHandle(CornerResizingHandle):
    def __init__(self, parent):
        super(BottomRightCornerResizingHandle, self).__init__(parent)

        self.setCursor(QtCore.Qt.SizeFDiagCursor)

    def performResizing(self, value):
        deltaX = value.x() - self.pos().x()
        deltaY = value.y() - self.pos().y()
        _, _, dx2, dy2 = self.parentResizable.performResizing(self, 0, 0, deltaX, deltaY)

        return QtCore.QPointF(dx2 + self.pos().x(), dy2 + self.pos().y())

class BottomLeftCornerResizingHandle(CornerResizingHandle):
    def __init__(self, parent):
        super(BottomLeftCornerResizingHandle, self).__init__(parent)

        self.setCursor(QtCore.Qt.SizeBDiagCursor)

    def performResizing(self, value):
        deltaX = value.x() - self.pos().x()
        deltaY = value.y() - self.pos().y()
        dx1, _, _, dy2 = self.parentResizable.performResizing(self, deltaX, 0, 0, deltaY)

        return QtCore.QPointF(dx1 + self.pos().x(), dy2 + self.pos().y())

class TopLeftCornerResizingHandle(CornerResizingHandle):
    def __init__(self, parent):
        super(TopLeftCornerResizingHandle, self).__init__(parent)

        self.setCursor(QtCore.Qt.SizeFDiagCursor)

    def performResizing(self, value):
        deltaX = value.x() - self.pos().x()
        deltaY = value.y() - self.pos().y()
        dx1, dy1, _, _ = self.parentResizable.performResizing(self, deltaX, deltaY, 0, 0)

        return QtCore.QPointF(dx1 + self.pos().x(), dy1 + self.pos().y())
*/

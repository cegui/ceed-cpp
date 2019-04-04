#include "src/ui/layout/LayoutManipulator.h"

LayoutManipulator::LayoutManipulator()
{
    setAcceptDrops(true);
/*
    def __init__(self, visual, parent, widget, recursive = True, skipAutoWidgets = False):
        self.visual = visual

        super(Manipulator, self).__init__(parent, widget, recursive, skipAutoWidgets)

        self.drawSnapGrid = False
        self.snapGridNonClientArea = False
        self.ignoreSnapGrid = False

        self.snapGridAction = action.getAction("layout/snap_grid")

        self.absoluteModeAction = action.getAction("layout/absolute_mode")
        self.absoluteModeAction.toggled.connect(self.slot_absoluteModeToggled)

        self.absoluteIntegersOnlyModeAction = action.getAction("layout/abs_integers_mode")
*/
}

LayoutManipulator::~LayoutManipulator()
{
/*
    def __del__(self):
        self.absoluteModeAction.toggled.disconnect(self.slot_absoluteModeToggled)
*/
}

void LayoutManipulator::setLocked(bool locked)
{
    setFlag(ItemIsMovable, !locked);
    setFlag(ItemIsSelectable, !locked);
    setFlag(ItemIsFocusable, !locked);

    setResizingEnabled(!locked);

    update();
}

/*
    snapGridBrush = None

    @classmethod
    def getSnapGridBrush(cls):
        """Retrieves a (cached) snap grid brush
        """

        snapGridX = settings.getEntry("layout/visual/snap_grid_x").value
        snapGridY = settings.getEntry("layout/visual/snap_grid_y").value
        snapGridPointColour = settings.getEntry("layout/visual/snap_grid_point_colour").value
        snapGridPointShadowColour = settings.getEntry("layout/visual/snap_grid_point_shadow_colour").value

        # if snap grid wasn't created yet or if it's parameters changed, create it anew!
        if (cls.snapGridBrush is None) or (cls.snapGridX != snapGridX) or (cls.snapGridY != snapGridY) or (cls.snapGridPointColour != snapGridPointColour) or (cls.snapGridPointShadowColour != snapGridPointShadowColour):
            cls.snapGridBrush = QtGui.QBrush()

            cls.snapGridX = snapGridX
            cls.snapGridY = snapGridY
            cls.snapGridPointColour = snapGridPointColour
            cls.snapGridPointShadowColour = snapGridPointShadowColour

            texture = QtGui.QPixmap(snapGridX, snapGridY)
            texture.fill(QtGui.QColor(QtCore.Qt.transparent))

            painter = QtGui.QPainter(texture)
            painter.setPen(QtGui.QPen(snapGridPointColour))
            painter.drawPoint(0, 0)
            painter.setPen(QtGui.QPen(snapGridPointShadowColour))
            painter.drawPoint(1, 0)
            painter.drawPoint(1, 1)
            painter.drawPoint(0, 1)
            painter.end()

            cls.snapGridBrush.setTexture(texture)

        return cls.snapGridBrush

    @staticmethod
    def getValidWidgetName(name):
        """Returns a valid CEGUI widget name out of the supplied name, if possible.
        Returns None if the supplied name is invalid and can't be converted to a valid name
        (an empty string for example).
        """
        if not name:
            return None
        name = name.strip()
        if not name:
            return None
        return name.replace("/", "_")


    def slot_absoluteModeToggled(self, checked):
        # immediately update if possible
        if self.resizeInProgress:
            self.notifyResizeProgress(self.lastResizeNewPos, self.lastResizeNewRect)
            self.update()

        if self.moveInProgress:
            self.notifyMoveProgress(self.lastMoveNewPos)
            self.update()

    def getNormalPen(self):
        return settings.getEntry("layout/visual/normal_outline").value if self.showOutline else QtGui.QColor(0, 0, 0, 0)

    def getHoverPen(self):
        return settings.getEntry("layout/visual/hover_outline").value if self.showOutline else QtGui.QColor(0, 0, 0, 0)

    def getPenWhileResizing(self):
        return settings.getEntry("layout/visual/resizing_outline").value

    def getPenWhileMoving(self):
        return settings.getEntry("layout/visual/moving_outline").value

    def getDragAcceptableHintPen(self):
        ret = QtGui.QPen()
        ret.setColor(QtGui.QColor(255, 255, 0))

        return ret

    def getUniqueChildWidgetName(self, base = "Widget"):
        """Finds a unique name for a child widget of the manipulated widget

        The resulting name's format is the base with a number appended
        """

        candidate = base

        if self.widget is None:
            # we can't check for duplicates in this case
            return candidate

        i = 2
        while self.widget.isChild(candidate):
            candidate = "%s%i" % (base, i)
            i += 1

        return candidate

    def createChildManipulator(self, childWidget, recursive = True, skipAutoWidgets = False):
        ret = Manipulator(self.visual, self, childWidget, recursive, skipAutoWidgets)
        ret.updateFromWidget()
        return ret

    def detach(self, detachWidget = True, destroyWidget = True, recursive = True):
        parentWidgetWasNone = self.widget.getParent() is None

        super(Manipulator, self).detach(detachWidget, destroyWidget, recursive)

        if parentWidgetWasNone:
            # if this was root we have to inform the scene accordingly!
            self.visual.setRootWidgetManipulator(None)

    def dragEnterEvent(self, event):
        if event.mimeData().hasFormat("application/x-ceed-widget-type"):
            event.acceptProposedAction()

            self.setPen(self.getDragAcceptableHintPen())

        else:
            event.ignore()

    def dragLeaveEvent(self, event):
        self.setPen(self.getNormalPen())

    def dropEvent(self, event):
        """Takes care of creating new widgets when user drops the right mime type here
        (dragging from the CreateWidgetDockWidget)
        """

        data = event.mimeData().data("application/x-ceed-widget-type")

        if data:
            widgetType = data.data()

            from ceed.editors.layout import undo
            cmd = undo.CreateCommand(self.visual, self.widget.getNamePath(), widgetType, self.getUniqueChildWidgetName(widgetType.rsplit("/", 1)[-1]))
            self.visual.tabbedEditor.undoStack.push(cmd)

            event.acceptProposedAction()

        else:
            event.ignore()

    def useAbsoluteCoordsForMove(self):
        return self.absoluteModeAction.isChecked()

    def useAbsoluteCoordsForResize(self):
        return self.absoluteModeAction.isChecked()

    def useIntegersForAbsoluteMove(self):
        return self.absoluteIntegersOnlyModeAction.isChecked()

    def useIntegersForAbsoluteResize(self):
        return self.absoluteIntegersOnlyModeAction.isChecked()

    def notifyResizeStarted(self):
        super(Manipulator, self).notifyResizeStarted()

        parent = self.parentItem()
        if isinstance(parent, Manipulator):
            parent.drawSnapGrid = True

    def notifyResizeProgress(self, newPos, newRect):
        super(Manipulator, self).notifyResizeProgress(newPos, newRect)

        self.triggerPropertyManagerCallback({"Size", "Position", "Area"})

    def notifyResizeFinished(self, newPos, newRect):
        super(Manipulator, self).notifyResizeFinished(newPos, newRect)

        parent = self.parentItem()
        if isinstance(parent, Manipulator):
            parent.drawSnapGrid = False

    def notifyMoveStarted(self):
        super(Manipulator, self).notifyMoveStarted()

        parent = self.parentItem()
        if isinstance(parent, Manipulator):
            parent.drawSnapGrid = True

    def notifyMoveProgress(self, newPos):
        super(Manipulator, self).notifyMoveProgress(newPos)

        self.triggerPropertyManagerCallback({"Position", "Area"})

    def notifyMoveFinished(self, newPos):
        super(Manipulator, self).notifyMoveFinished(newPos)

        parent = self.parentItem()
        if isinstance(parent, Manipulator):
            parent.drawSnapGrid = False

    def getPreventManipulatorOverlap(self):
        """Returns whether the painting code should strive to prevent manipulator overlap (crossing outlines and possibly other things)
        Override to change the behavior
        """

        return settings.getEntry("layout/visual/prevent_manipulator_overlap").value

    def impl_paint(self, painter, option, widget):
        super(Manipulator, self).impl_paint(painter, option, widget)

        if self.drawSnapGrid and self.snapGridAction.isChecked():
            childRect = self.widget.getChildContentArea(self.snapGridNonClientArea).get()
            qChildRect = QtCore.QRectF(childRect.d_min.d_x, childRect.d_min.d_y, childRect.getWidth(), childRect.getHeight())
            qChildRect.translate(-self.scenePos())

            painter.save()
            painter.setBrushOrigin(qChildRect.topLeft())
            painter.fillRect(qChildRect, Manipulator.getSnapGridBrush())
            painter.restore()

    def updateFromWidget(self, callUpdate = False, updateAncestorLCs = False):
        # we are updating the position and size from widget, we don't want any snapping
        self.ignoreSnapGrid = True
        super(Manipulator, self).updateFromWidget(callUpdate, updateAncestorLCs)
        self.ignoreSnapGrid = False

        self.showOutline = True
        self.setFlags(self.flags() | QtGui.QGraphicsItem.ItemIsFocusable)
        self.setFlags(self.flags() | QtGui.QGraphicsItem.ItemIsSelectable)
        self.setFlags(self.flags() | QtGui.QGraphicsItem.ItemIsMovable)
        self.setFlags(self.flags() & ~QtGui.QGraphicsItem.ItemHasNoContents)
        self.setResizingEnabled(True)

        if self.widget.isAutoWindow():
            if not settings.getEntry("layout/visual/auto_widgets_show_outline").value:
                # don't show outlines unless instructed to do so
                self.showOutline = False

            if not settings.getEntry("layout/visual/auto_widgets_selectable").value:
                # make this widget not focusable, selectable, movable and resizable
                self.setFlags(self.flags() & ~QtGui.QGraphicsItem.ItemIsFocusable)
                self.setFlags(self.flags() & ~QtGui.QGraphicsItem.ItemIsSelectable)
                self.setFlags(self.flags() & ~QtGui.QGraphicsItem.ItemIsMovable)
                self.setFlags(self.flags() |  QtGui.QGraphicsItem.ItemHasNoContents)
                self.setResizingEnabled(False)

        if isinstance(self.widget, PyCEGUI.LayoutContainer):
            # LayoutContainers change their size to fit the widgets, it makes
            # no sense to show this size
            self.showOutline = False
            # And it makes no sense to resize them, they will just snap back
            # when they relayout
            self.setResizingEnabled(False)

        parent = self.widget.getParent()
        if parent and isinstance(parent, PyCEGUI.LayoutContainer):
            # if the widget is now parented inside a layout container we don't want
            # any drag moving to be possible
            self.setFlags(self.flags() & ~QtGui.QGraphicsItem.ItemIsMovable)

    def snapXCoordToGrid(self, x):
        # we have to take the child rect into account
        childRect = self.widget.getChildContentArea(self.snapGridNonClientArea).get()
        xOffset = childRect.d_min.d_x - self.scenePos().x()

        # point is in local space
        snapGridX = settings.getEntry("layout/visual/snap_grid_x").value
        return xOffset + round((x - xOffset) / snapGridX) * snapGridX

    def snapYCoordToGrid(self, y):
        # we have to take the child rect into account
        childRect = self.widget.getChildContentArea(self.snapGridNonClientArea).get()
        yOffset = childRect.d_min.d_y - self.scenePos().y()

        # point is in local space
        snapGridY = settings.getEntry("layout/visual/snap_grid_y").value
        return yOffset + round((y - yOffset) / snapGridY) * snapGridY

    def constrainMovePoint(self, point):
        if not self.ignoreSnapGrid and hasattr(self, "snapGridAction") and self.snapGridAction.isChecked():
            parent = self.parentItem()
            if parent is None:
                # ad hoc snapping for root widget, it snaps to itself
                parent = self

            if isinstance(parent, Manipulator):
                point = QtCore.QPointF(parent.snapXCoordToGrid(point.x()), parent.snapYCoordToGrid(point.y()))

        point = super(Manipulator, self).constrainMovePoint(point)

        return point

    def constrainResizeRect(self, rect, oldRect):
        # we constrain all 4 "corners" to the snap grid if needed
        if not self.ignoreSnapGrid and hasattr(self, "snapGridAction") and self.snapGridAction.isChecked():
            parent = self.parentItem()
            if parent is None:
                # ad hoc snapping for root widget, it snaps to itself
                parent = self

            if isinstance(parent, Manipulator):
                # we only snap the coordinates that have changed
                # because for example when you drag the left edge you don't want the right edge to snap!

                # we have to add the position coordinate as well to ensure the snap is precisely at the guide point
                # it is subtracted later on because the rect is relative to the item position

                if rect.left() != oldRect.left():
                    rect.setLeft(parent.snapXCoordToGrid(self.pos().x() + rect.left()) - self.pos().x())
                if rect.top() != oldRect.top():
                    rect.setTop(parent.snapYCoordToGrid(self.pos().y() + rect.top()) - self.pos().y())

                if rect.right() != oldRect.right():
                    rect.setRight(parent.snapXCoordToGrid(self.pos().x() + rect.right()) - self.pos().x())
                if rect.bottom() != oldRect.bottom():
                    rect.setBottom(parent.snapYCoordToGrid(self.pos().y() + rect.bottom()) - self.pos().y())

        rect = super(Manipulator, self).constrainResizeRect(rect, oldRect)

        return rect
*/

#include "src/ui/layout/LayoutManipulator.h"
#include "src/util/Settings.h"
#include "src/Application.h"
#include "qpen.h"
#include "qgraphicssceneevent.h"
#include "qmimedata.h"

// Returns a valid CEGUI widget name out of the supplied name, if possible. Returns empty string if
// the supplied name is invalid and can't be converted to a valid name (an empty string for example).
QString LayoutManipulator::getValidWidgetName(const QString& name)
{
    QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) return "";
    return trimmed.replace("/", "_");
}

LayoutManipulator::LayoutManipulator(LayoutVisualMode& visualMode, QGraphicsItem* parent, bool recursive, bool skipAutoWidgets)
    : CEGUIManipulator(parent, recursive, skipAutoWidgets)
    , _visualMode(visualMode)
{
    setAcceptDrops(true);
/*
    def __init__(self, visual, parent, widget, recursive = True, skipAutoWidgets = False):
        super(Manipulator, self).__init__(parent, widget, recursive, skipAutoWidgets)

        self.snapGridAction = action.getAction("layout/snap_grid")

        self.absoluteModeAction = action.getAction("layout/absolute_mode")
        self.absoluteModeAction.toggled.connect(self.slot_absoluteModeToggled)

        self.absoluteIntegersOnlyModeAction = action.getAction("layout/abs_integers_mode")
*/
}

LayoutManipulator::~LayoutManipulator()
{
}

void LayoutManipulator::getChildLayoutManipulators(std::vector<LayoutManipulator*>& outList, bool recursive)
{
    for (QGraphicsItem* item : childItems())
    {
        LayoutManipulator* manipulator = dynamic_cast<LayoutManipulator*>(item);
        if (manipulator)
        {
            outList.push_back(manipulator);
            if (recursive) manipulator->getChildLayoutManipulators(outList, true);
        }
    }
}

QPointF LayoutManipulator::constrainMovePoint(QPointF value)
{
/*
    if not self.ignoreSnapGrid and hasattr(self, "snapGridAction") and self.snapGridAction.isChecked()
    {
        parent = self.parentItem()
        if parent is None:
            # ad hoc snapping for root widget, it snaps to itself
            parent = self

        if isinstance(parent, Manipulator):
            point = QtCore.QPointF(parent.snapXCoordToGrid(point.x()), parent.snapYCoordToGrid(point.y()))
    }
*/
    return CEGUIManipulator::constrainMovePoint(value);
}

QRectF LayoutManipulator::constrainResizeRect(QRectF rect, QRectF oldRect)
{
/*
    // We constrain all 4 "corners" to the snap grid if needed
    if not self.ignoreSnapGrid and hasattr(self, "snapGridAction") and self.snapGridAction.isChecked()
    {
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
    }
*/
    return CEGUIManipulator::constrainResizeRect(rect, oldRect);
}

void LayoutManipulator::notifyResizeStarted(ResizingHandle* handle)
{
    CEGUIManipulator::notifyResizeStarted(handle);

    LayoutManipulator* parentManipulator = dynamic_cast<LayoutManipulator*>(parentItem());
    if (parentManipulator) parentManipulator->_drawSnapGrid = true;
}

void LayoutManipulator::notifyResizeProgress(QPointF newPos, QRectF newRect)
{
    CEGUIManipulator::notifyResizeProgress(newPos, newRect);
    triggerPropertyManagerCallback({"Size", "Position", "Area"});
}

void LayoutManipulator::notifyResizeFinished(QPointF newPos, QRectF newRect)
{
    CEGUIManipulator::notifyResizeFinished(newPos, newRect);

    LayoutManipulator* parentManipulator = dynamic_cast<LayoutManipulator*>(parentItem());
    if (parentManipulator) parentManipulator->_drawSnapGrid = false;
}

void LayoutManipulator::notifyMoveStarted()
{
    CEGUIManipulator::notifyMoveStarted();

    LayoutManipulator* parentManipulator = dynamic_cast<LayoutManipulator*>(parentItem());
    if (parentManipulator) parentManipulator->_drawSnapGrid = true;
}

void LayoutManipulator::notifyMoveProgress(QPointF newPos)
{
    CEGUIManipulator::notifyMoveProgress(newPos);
    triggerPropertyManagerCallback({"Position", "Area"});
}

void LayoutManipulator::notifyMoveFinished(QPointF newPos)
{
    CEGUIManipulator::notifyMoveFinished(newPos);

    LayoutManipulator* parentManipulator = dynamic_cast<LayoutManipulator*>(parentItem());
    if (parentManipulator) parentManipulator->_drawSnapGrid = false;
}

void LayoutManipulator::updateFromWidget(bool callUpdate, bool updateAncestorLCs)
{
    // We are updating the position and size from widget, we don't want any snapping
    _ignoreSnapGrid = true;
    CEGUIManipulator::updateFromWidget(callUpdate, updateAncestorLCs);
    _ignoreSnapGrid = false;

    _showOutline = true;

    auto currFlags = flags();
    currFlags |= (ItemIsFocusable | ItemIsSelectable | ItemIsMovable);
    currFlags &= ~ItemHasNoContents;
    setFlags(currFlags);

    setResizingEnabled(true);

/*
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
*/
}

void LayoutManipulator::detach(bool detachWidget, bool destroyWidget, bool recursive)
{
/*
    parentWidgetWasNone = self.widget.getParent() is None
*/
    CEGUIManipulator::detach(detachWidget, destroyWidget, recursive);
/*
    if parentWidgetWasNone:
        # if this was root we have to inform the scene accordingly!
        self.visual.setRootWidgetManipulator(None)
*/
}

bool LayoutManipulator::preventManipulatorOverlap() const
{
    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();
    return settings->getEntryValue("layout/visual/prevent_manipulator_overlap").toBool();
}

bool LayoutManipulator::useAbsoluteCoordsForMove() const
{
    /*
        return self.absoluteModeAction.isChecked()
    */
    return false;
}

bool LayoutManipulator::useAbsoluteCoordsForResize() const
{
    /*
        return self.absoluteModeAction.isChecked()
    */
    return false;
}

bool LayoutManipulator::useIntegersForAbsoluteMove() const
{
    /*
        return self.absoluteIntegersOnlyModeAction.isChecked()
    */
    return false;
}

bool LayoutManipulator::useIntegersForAbsoluteResize() const
{
    /*
        return self.absoluteIntegersOnlyModeAction.isChecked()
    */
    return false;
}

void LayoutManipulator::setLocked(bool locked)
{
    setFlag(ItemIsMovable, !locked);
    setFlag(ItemIsSelectable, !locked);
    setFlag(ItemIsFocusable, !locked);

    setResizingEnabled(!locked);

    update();
}

void LayoutManipulator::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
{
    if (event->mimeData()->hasFormat("application/x-ceed-widget-type"))
    {
        event->acceptProposedAction();
        setPen(QPen(QColor(255, 255, 0)));
    }
    else
    {
        event->ignore();
    }
}

void LayoutManipulator::dragLeaveEvent(QGraphicsSceneDragDropEvent* event)
{
    setPen(getNormalPen());
}

// Takes care of creating new widgets when user drops the right mime type here
// (dragging from the CreateWidgetDockWidget)
void LayoutManipulator::dropEvent(QGraphicsSceneDragDropEvent* event)
{
    auto data = event->mimeData()->data("application/x-ceed-widget-type");
    if (data.size() > 0)
    {
        QString widgetType = data.data();
        /*
            cmd = undo.CreateCommand(self.visual, self.widget.getNamePath(), widgetType, self.getUniqueChildWidgetName(widgetType.rsplit("/", 1)[-1]))
            self.visual.tabbedEditor.undoStack.push(cmd)
        */
        event->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
}

QPen LayoutManipulator::getNormalPen() const
{
    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();
    return _showOutline ? settings->getEntryValue("layout/visual/normal_outline").value<QPen>() : QPen(QColor(0, 0, 0, 0));
}

QPen LayoutManipulator::getHoverPen() const
{
    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();
    return _showOutline ? settings->getEntryValue("layout/visual/hover_outline").value<QPen>() : QPen(QColor(0, 0, 0, 0));
}

QPen LayoutManipulator::getPenWhileResizing() const
{
    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();
    return settings->getEntryValue("layout/visual/resizing_outline").value<QPen>();
}

QPen LayoutManipulator::getPenWhileMoving() const
{
    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();
    return settings->getEntryValue("layout/visual/moving_outline").value<QPen>();
}

void LayoutManipulator::impl_paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    CEGUIManipulator::impl_paint(painter, option, widget);
/*
        if self.drawSnapGrid and self.snapGridAction.isChecked():
            childRect = self.widget.getChildContentArea(self.snapGridNonClientArea).get()
            qChildRect = QtCore.QRectF(childRect.d_min.d_x, childRect.d_min.d_y, childRect.getWidth(), childRect.getHeight())
            qChildRect.translate(-self.scenePos())

            painter.save()
            painter.setBrushOrigin(qChildRect.topLeft())
            painter.fillRect(qChildRect, Manipulator.getSnapGridBrush())
            painter.restore()
*/
}

// Finds a unique name for a child widget of the manipulated widget.
// The resulting name's format is the base with a number appended.
QString LayoutManipulator::getUniqueChildWidgetName(const QString& base)
{
    QString candidate = base;
/*
        if self.widget is None:
            // We can't check for duplicates in this case
            return candidate

        i = 2
        while self.widget.isChild(candidate):
            candidate = "%s%i" % (base, i)
            i += 1
*/
    return candidate;
}

qreal LayoutManipulator::snapXCoordToGrid(qreal x)
{
    // We have to take the child rect into account
/*
        childRect = self.widget.getChildContentArea(self.snapGridNonClientArea).get()
        xOffset = childRect.d_min.d_x - self.scenePos().x()

        # point is in local space
        snapGridX = settings.getEntry("layout/visual/snap_grid_x").value
        return xOffset + round((x - xOffset) / snapGridX) * snapGridX
*/
    return x;
}

qreal LayoutManipulator::snapYCoordToGrid(qreal y)
{
    // We have to take the child rect into account
/*
        childRect = self.widget.getChildContentArea(self.snapGridNonClientArea).get()
        yOffset = childRect.d_min.d_y - self.scenePos().y()

        # point is in local space
        snapGridY = settings.getEntry("layout/visual/snap_grid_y").value
        return yOffset + round((y - yOffset) / snapGridY) * snapGridY
*/
    return y;
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


    def slot_absoluteModeToggled(self, checked):
        # immediately update if possible
        if self.resizeInProgress:
            self.notifyResizeProgress(self.lastResizeNewPos, self.lastResizeNewRect)
            self.update()

        if self.moveInProgress:
            self.notifyMoveProgress(self.lastMoveNewPos)
            self.update()

    def createChildManipulator(self, childWidget, recursive = True, skipAutoWidgets = False):
        ret = Manipulator(self.visual, self, childWidget, recursive, skipAutoWidgets)
        ret.updateFromWidget()
        return ret
*/

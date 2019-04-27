#include "src/ui/layout/LayoutManipulator.h"
#include "src/editors/layout/LayoutVisualMode.h"
#include "src/util/Settings.h"
#include "src/Application.h"
#include "qpen.h"
#include "qgraphicssceneevent.h"
#include "qmimedata.h"
#include "qaction.h"
#include <CEGUI/Window.h>

// Returns a valid CEGUI widget name out of the supplied name, if possible. Returns empty string if
// the supplied name is invalid and can't be converted to a valid name (an empty string for example).
QString LayoutManipulator::getValidWidgetName(const QString& name)
{
    QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) return "";
    return trimmed.replace("/", "_");
}

LayoutManipulator::LayoutManipulator(LayoutVisualMode& visualMode, QGraphicsItem* parent, CEGUI::Window* widget, bool recursive, bool skipAutoWidgets)
    : CEGUIManipulator(parent, widget, recursive, skipAutoWidgets)
    , _visualMode(visualMode)
{
    setAcceptDrops(true);
    QObject::connect(_visualMode.getAbsoluteModeAction(), &QAction::toggled, [this]
    {
        // Immediately update if possible
        if (_resizeInProgress)
        {
            notifyResizeProgress(_lastResizeNewPos, _lastResizeNewRect);
            update();
        }
        if (_moveInProgress)
        {
            notifyMoveProgress(_lastMoveNewPos);
            update();
        }
    });
}

LayoutManipulator::~LayoutManipulator()
{
}

LayoutManipulator* LayoutManipulator::createChildManipulator(CEGUI::Window* childWidget, bool recursive, bool skipAutoWidgets)
{
    auto ret = new LayoutManipulator(_visualMode, this, childWidget, recursive, skipAutoWidgets);
    ret->updateFromWidget();
    return ret;
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
    if (!_ignoreSnapGrid && _visualMode.isSnapGridEnabled())
    {
        auto parent = parentItem();
        if (!parent) parent = this; // Ad hoc snapping for root widget, it snaps to itself

        auto parentManip = dynamic_cast<LayoutManipulator*>(parent);
        if (parentManip)
            value = QPointF(parentManip->snapXCoordToGrid(value.x()), parentManip->snapYCoordToGrid(value.y()));
    }

    return CEGUIManipulator::constrainMovePoint(value);
}

static inline bool compareReal(qreal a, qreal b) { return std::abs(a - b) < static_cast<qreal>(0.0001); }

QRectF LayoutManipulator::constrainResizeRect(QRectF rect, QRectF oldRect)
{
    // We constrain all 4 "corners" to the snap grid if needed
    if (!_ignoreSnapGrid && _visualMode.isSnapGridEnabled())
    {
        auto parent = parentItem();
        if (!parent) parent = this; // Ad hoc snapping for root widget, it snaps to itself

        auto parentManip = dynamic_cast<LayoutManipulator*>(parent);
        if (parentManip)
        {
            // We only snap the coordinates that have changed
            // because for example when you drag the left edge you don't want the right edge to snap!
            // We have to add the position coordinate as well to ensure the snap is precisely at the guide point
            // it is subtracted later on because the rect is relative to the item position
            if (!compareReal(rect.left(), oldRect.left()))
                rect.setLeft(parentManip->snapXCoordToGrid(pos().x() + rect.left()) - pos().x());
            if (!compareReal(rect.top(), oldRect.top()))
                rect.setTop(parentManip->snapYCoordToGrid(pos().y() + rect.top()) - pos().y());
            if (!compareReal(rect.right(), oldRect.right()))
                rect.setRight(parentManip->snapXCoordToGrid(pos().x() + rect.right()) - pos().x());
            if (!compareReal(rect.bottom(), oldRect.bottom()))
                rect.setBottom(parentManip->snapYCoordToGrid(pos().y() + rect.bottom()) - pos().y());
        }
    }

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
    const bool parentWidgetWasNone = _widget->getParent();

    CEGUIManipulator::detach(detachWidget, destroyWidget, recursive);

    // If this was root we have to inform the scene accordingly!
    if (parentWidgetWasNone) _visualMode.setRootWidgetManipulator(nullptr);
}

bool LayoutManipulator::preventManipulatorOverlap() const
{
    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();
    return settings->getEntryValue("layout/visual/prevent_manipulator_overlap").toBool();
}

bool LayoutManipulator::useAbsoluteCoordsForMove() const
{
    return _visualMode.isAbsoluteMode();
}

bool LayoutManipulator::useAbsoluteCoordsForResize() const
{
    return _visualMode.isAbsoluteMode();
}

bool LayoutManipulator::useIntegersForAbsoluteMove() const
{
    return _visualMode.isAbsoluteIntegerMode();
}

bool LayoutManipulator::useIntegersForAbsoluteResize() const
{
    return _visualMode.isAbsoluteIntegerMode();
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

    if (_drawSnapGrid && _visualMode.isSnapGridEnabled())
    {
        /*
            childRect = self.widget.getChildContentArea(self.snapGridNonClientArea).get()
            qChildRect = QtCore.QRectF(childRect.d_min.d_x, childRect.d_min.d_y, childRect.getWidth(), childRect.getHeight())
            qChildRect.translate(-self.scenePos())

            painter.save()
            painter.setBrushOrigin(qChildRect.topLeft())
            painter.fillRect(qChildRect, _visualMode.getSnapGridBrush())
            painter.restore()
        */
    }
}

// Finds a unique name for a child widget of the manipulated widget.
// The resulting name's format is the base with a number appended.
QString LayoutManipulator::getUniqueChildWidgetName(const QString& base) const
{
    // We can't check for duplicates in this case
    if (!_widget) return base;

    QString candidate = base;
    int i = 2;
    while (_widget->isChild(candidate.toLocal8Bit().data()))
    {
        candidate = base + QString::number(i);
        ++i;
    }

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

#include "src/ui/layout/LayoutManipulator.h"
#include "src/ui/layout/LayoutScene.h"
#include "src/ui/layout/LayoutContainerHandle.h"
#include "src/ui/layout/WidgetHierarchyItem.h"
#include "src/editors/layout/LayoutVisualMode.h"
#include "src/editors/layout/LayoutUndoCommands.h"
#include "src/cegui/CEGUIUtils.h"
#include "src/util/Settings.h"
#include "src/Application.h"
#include <CEGUI/widgets/LayoutContainer.h>
#include "QtnProperty/PropertySet.h"
#include "qpen.h"
#include "qpainter.h"
#include "qgraphicssceneevent.h"
#include "qmimedata.h"
#include "qaction.h"
#include "qmessagebox.h"

LayoutManipulator::LayoutManipulator(LayoutVisualMode& visualMode, QGraphicsItem* parent, CEGUI::Window* widget)
    : CEGUIManipulator(parent, widget)
    , _visualMode(visualMode)
{
    resetPen(); // We override the pen so we must set it in the constructor
    setAcceptDrops(true);

    if (isLayoutContainer())
        _lcHandle = new LayoutContainerHandle(*this);

    QObject::connect(_visualMode.getAbsoluteModeAction(), &QAction::toggled, [this]
    {
        // Immediately update if possible
        if (_resizeInProgress)
            notifyResizeProgress(_lastNewPos, _lastNewSize);
        if (_moveInProgress)
            notifyMoveProgress(_lastNewPos);
        if (_resizeInProgress || _moveInProgress)
            update();
    });
}

LayoutManipulator::~LayoutManipulator()
{
}

LayoutManipulator* LayoutManipulator::createChildManipulator(CEGUI::Window* childWidget)
{
    return new LayoutManipulator(_visualMode, this, childWidget);
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

// qFuzzyCompare doesn't work with doubles when one of them may be 0.0
// See docs: https://doc.qt.io/qt-5/qtglobal.html#qFuzzyCompare
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

void LayoutManipulator::notifyResizeStarted()
{
    CEGUIManipulator::notifyResizeStarted();

    LayoutManipulator* parentManipulator = dynamic_cast<LayoutManipulator*>(parentItem());
    if (parentManipulator) parentManipulator->_drawSnapGrid = true;
}

void LayoutManipulator::notifyResizeProgress(QPointF newPos, QSizeF newSize)
{
    CEGUIManipulator::notifyResizeProgress(newPos, newSize);
    _lastNewPos = newPos;
    _lastNewSize = newSize;
}

void LayoutManipulator::notifyResizeFinished(QPointF newPos, QSizeF newSize)
{
    CEGUIManipulator::notifyResizeFinished(newPos, newSize);

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
    _lastNewPos = newPos;
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

    auto currFlags = flags();
    currFlags |= (ItemIsFocusable | ItemIsSelectable | ItemIsMovable);
    currFlags &= ~ItemHasNoContents;

    _showOutline = true;
    _resizeable = true;
    if (_widget->isAutoWindow())
    {
        auto&& settings = qobject_cast<Application*>(qApp)->getSettings();

        // Don't show outlines unless instructed to do so
        if (!settings->getEntryValue("layout/visual/auto_widgets_show_outline").toBool())
            _showOutline = false;

        if (!settings->getEntryValue("layout/visual/auto_widgets_selectable").toBool())
        {
            // Make this widget non-interactive
            currFlags |= ItemHasNoContents;
            currFlags &= ~(ItemIsFocusable | ItemIsSelectable | ItemIsMovable);
            _resizeable = false;
        }
    }

    if (isLayoutContainer())
    {
        // It makes no sense to resize LCs, they will just snap back when they relayout
        _resizeable = false;
    }

    // FIXME LC: drag inside sequential layouts at least
    // Only size handles (right, bottom) and anchors
    // If the widget is parented inside a layout container we don't want any drag moving to be possible
    if (dynamic_cast<CEGUI::LayoutContainer*>(_widget->getParent()))
        currFlags &= ~ItemIsMovable;

    setFlags(currFlags);
    setResizingEnabled(_resizeable);

    _visualMode.getScene()->onManipulatorUpdatedFromWidget(this);
}

void LayoutManipulator::detach(bool detachWidget, bool destroyWidget, bool recursive)
{
    const bool isRoot = (_visualMode.getScene()->getRootWidgetManipulator() == this);

    CEGUIManipulator::detach(detachWidget, destroyWidget, recursive);

    // If this was root we have to inform the scene accordingly!
    if (isRoot) _visualMode.setRootWidgetManipulator(nullptr);
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

bool LayoutManipulator::renameWidget(QString& newName)
{
    if (newName == getWidgetName()) return true;

    // Validate the new name, cancel if invalid
    newName = CEGUIUtils::getValidWidgetName(newName);
    if (newName.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText("The name was not changed because the new name is invalid.");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        return false;
    }

    // Check if the new name is unique in the parent, cancel if not
    auto parentWidget = getWidget()->getParent();
    if (parentWidget && parentWidget->isChild(CEGUIUtils::qStringToString(newName)))
    {
        QMessageBox msgBox;
        msgBox.setText("The name was not changed because the new name is in use by a sibling widget.");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        return false;
    }

    // The name is good, apply it
    _visualMode.getEditor().getUndoStack()->push(new LayoutRenameCommand(_visualMode, getWidgetPath(), newName));
    return true;
}

void LayoutManipulator::setLocked(bool locked)
{
    setFlag(ItemIsMovable, !locked);
    setFlag(ItemIsSelectable, !locked);
    setFlag(ItemIsFocusable, !locked);

    setResizingEnabled(_resizeable && !locked);

    update();
}

void LayoutManipulator::resetPen()
{
    setPen(getNormalPen());
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

void LayoutManipulator::dragLeaveEvent(QGraphicsSceneDragDropEvent* /*event*/)
{
    resetPen();
}

// Takes care of creating new widgets when user drops the right mime type here
// (dragging from the CreateWidgetDockWidget)
void LayoutManipulator::dropEvent(QGraphicsSceneDragDropEvent* event)
{
    auto data = event->mimeData()->data("application/x-ceed-widget-type");
    if (data.size() > 0)
    {
        QString widgetType = data.data();
        int sepPos = widgetType.lastIndexOf('/');
        QString widgetName = CEGUIUtils::getUniqueChildWidgetName(*_widget, (sepPos < 0) ? widgetType : widgetType.mid(sepPos + 1));
        _visualMode.getEditor().getUndoStack()->push(new LayoutCreateCommand(_visualMode, getWidgetPath(), widgetType, widgetName, event->scenePos()));
        event->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
}

QVariant LayoutManipulator::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemSelectedHasChanged)
    {
        if (_lcHandle)
        {
            if (value.toBool())
                _lcHandle->setSelected(false);
            else
                _lcHandle->updateLook();
        }
    }

    return CEGUIManipulator::itemChange(change, value);
}

void LayoutManipulator::onPropertyChanged(const QtnPropertyBase* property, CEGUI::Property* ceguiProperty)
{
    QString value;
    if (property->toStr(value))
    {
        if (ceguiProperty->getName() == "Name")
        {
            // Special case: when we edit the name, widget path changes and LayoutPropertyEditCommand
            // will fail to find the widget. Use LayoutRenameCommand here. Also it is more consistent.
            QString newName = value;
            if (!renameWidget(newName))
                newName = getWidgetName();

            // Set actual name back to the property
            if (newName != value)
                updatePropertiesFromWidget({"Name", "NamePath"});
        }
        else
        {
            std::vector<LayoutPropertyEditCommand::Record> records;

            LayoutPropertyEditCommand::Record rec;
            rec.path = getWidgetPath();
            rec.oldValue = ceguiProperty->get(_widget);
            rec.newValue = CEGUIUtils::qStringToString(value);
            records.push_back(std::move(rec));

            // Handle multiproperty merge in a command itself
            const size_t groupId = _visualMode.getScene()->getMultiSelectionChangeId();
            _visualMode.getEditor().getUndoStack()->push(new LayoutPropertyEditCommand(_visualMode, std::move(records), property->name(), groupId));
        }
    }
}

void LayoutManipulator::onWidgetNameChanged()
{
    CEGUIManipulator::onWidgetNameChanged();
    if (_treeItem) _treeItem->refreshPathData();
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
        const auto& childRect = _widget->getChildContentArea(_snapGridNonClientArea).get();
        QRectF qChildRect(static_cast<qreal>(childRect.d_min.x),
                          static_cast<qreal>(childRect.d_min.y),
                          static_cast<qreal>(childRect.getWidth()),
                          static_cast<qreal>(childRect.getHeight()));
         qChildRect.translate(-scenePos());

        painter->save();
        painter->setBrushOrigin(qChildRect.topLeft());
        painter->fillRect(qChildRect, _visualMode.getSnapGridBrush());
        painter->restore();
    }
}

qreal LayoutManipulator::snapXCoordToGrid(qreal x)
{
    // We have to take the child rect into account
    const auto& childRect = _widget->getChildContentArea(_snapGridNonClientArea).get();
    const qreal xOffset = static_cast<qreal>(childRect.d_min.x) - scenePos().x();

    // Point is in local space
    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();
    const int snapGridX = settings->getEntryValue("layout/visual/snap_grid_x").toInt();
    return xOffset + round((x - xOffset) / snapGridX) * snapGridX;
}

qreal LayoutManipulator::snapYCoordToGrid(qreal y)
{
    // We have to take the child rect into account
    const auto& childRect = _widget->getChildContentArea(_snapGridNonClientArea).get();
    const qreal yOffset = static_cast<qreal>(childRect.d_min.y) - scenePos().y();

    // Point is in local space
    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();
    const int snapGridY = settings->getEntryValue("layout/visual/snap_grid_y").toInt();
    return yOffset + round((y - yOffset) / snapGridY) * snapGridY;
}

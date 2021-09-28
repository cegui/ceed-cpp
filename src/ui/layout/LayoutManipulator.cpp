#include "src/ui/layout/LayoutManipulator.h"
#include "src/ui/layout/LayoutScene.h"
#include "src/ui/layout/LayoutContainerHandle.h"
#include "src/ui/layout/WidgetHierarchyItem.h"
#include "src/editors/layout/LayoutVisualMode.h"
#include "src/editors/layout/LayoutUndoCommands.h"
#include "src/cegui/CEGUIUtils.h"
#include "src/util/Settings.h"
#include "src/Application.h"
#include <CEGUI/widgets/GridLayoutContainer.h>
#include <CEGUI/widgets/TabControl.h>
#include "QtnProperty/PropertySet.h"
#include "qpen.h"
#include "qpainter.h"
#include <qgraphicssceneevent.h>
#include "qmimedata.h"
#include "qaction.h"
#include "qmessagebox.h"
#include <qdrag.h>
#include <qtimer.h>

void LayoutManipulator::removeNestedManipulators(std::set<LayoutManipulator*>& manipulators)
{
    for (auto it = manipulators.begin(); it != manipulators.end(); /**/)
    {
        bool erased = false;
        const LayoutManipulator* manipulator = *it;
        for (LayoutManipulator* potentialParent : manipulators)
            if (potentialParent->isAncestorOf(manipulator))
            {
                it = manipulators.erase(it);
                erased = true;
                break;
            }

        if (!erased) ++it;
    }
}

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

        if (auto parentManip = dynamic_cast<LayoutManipulator*>(parent))
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

        if (auto parentManip = dynamic_cast<LayoutManipulator*>(parent))
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

    if (auto parentManipulator = dynamic_cast<LayoutManipulator*>(parentItem()))
        parentManipulator->_drawSnapGrid = true;
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

    if (auto parentManipulator = dynamic_cast<LayoutManipulator*>(parentItem()))
        parentManipulator->_drawSnapGrid = false;
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
    currFlags &= ~(ItemHasNoContents | ItemStacksBehindParent);

    _showOutline = true;
    _resizeable = true;
    bool hoverable = true;
    if (_widget->isAutoWindow())
    {
        auto&& settings = qobject_cast<Application*>(qApp)->getSettings();

        // Don't show outlines unless instructed to do so
        if (!settings->getEntryValue("layout/visual/auto_widgets_show_outline").toBool())
            _showOutline = false;

        if (!settings->getEntryValue("layout/visual/auto_widgets_selectable").toBool())
        {
            // Make this widget non-interactive
            currFlags |= (ItemHasNoContents | ItemStacksBehindParent);
            currFlags &= ~(ItemIsFocusable | ItemIsSelectable | ItemIsMovable);
            _resizeable = false;
            hoverable = false;
        }
    }

    setAcceptHoverEvents(hoverable);

    const bool isInTabCtl = isInTabControl();

    // It makes no sense to resize LCs, they will just snap back when they relayout.
    // TabControl tabs should not be resizeable.
    if (isInTabCtl || isLayoutContainer())
        _resizeable = false;

    // If the widget is parented inside a layout container we don't want any drag moving to be possible
    // TODO: can add drag reordering inside layout containers (https://github.com/cegui/ceed-cpp/issues/53)
    // TabControl tabs should not be movable.
    if (isInTabCtl || isInLayoutContainer())
        currFlags &= ~ItemIsMovable;

    // Allow to interact with the TabControl itself
    if (isInTabCtl)
    {
        // FIXME: RMB click still selects an item, WTF?
        // setVisible(false) prevents dropping child widgets into the tab and also tab deletion
        setAcceptedMouseButtons(Qt::NoButton);
        //currFlags &= ~ItemIsSelectable;
        //currFlags &= ~ItemIsFocusable;

        // To access TabControl resizing handles
        setZValue(-1.0);
    }

    // Move the active tab of the TabControl to front
    if (auto tabCtl = dynamic_cast<CEGUI::TabControl*>(_widget))
    {
        if (tabCtl->getTabCount())
        {
            // Reselect tab to update visibility. CEED deserialization may load it when it is not desired.
            // TODO CEGUI: add selected tab as a TabColtrol's property instead, CEED will restore it when deserialize!
            tabCtl->setSelectedTabAtIndex(tabCtl->getSelectedTabIndex());
            QString tabPath = CEGUIUtils::getRelativePath(tabCtl->getTabContentsAtIndex(tabCtl->getSelectedTabIndex()), tabCtl);
            if (auto tab = getManipulatorByPath(tabPath))
                tab->moveToFront();
        }
    }

    setFlags(currFlags);
    setResizingEnabled(_resizeable);

    _visualMode.getScene()->onManipulatorUpdatedFromWidget(this);
}

void LayoutManipulator::detach(bool detachWidget, bool destroyWidget, bool recursive)
{
    const bool isRoot = (_visualMode.getScene()->getRootWidgetManipulator() == this);

    CEGUIManipulator::detach(detachWidget, destroyWidget, recursive);

    if (_treeItem)
    {
        auto index = _treeItem->index();
        _treeItem->model()->removeRow(index.row(), index.parent());
        _treeItem = nullptr;
    }

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

void LayoutManipulator::showLayoutContainerHandles(bool show)
{
    if (_lcHandle) _lcHandle->setVisible(show);
    for (QGraphicsItem* child : childItems())
        if (auto manipulator = dynamic_cast<LayoutManipulator*>(child))
            manipulator->showLayoutContainerHandles(show);
}

void LayoutManipulator::updateHandles()
{
    if (_lcHandle && _handlesDirty && !_moveInProgress)
        _lcHandle->updatePositionAndScale(_currentScaleX, _currentScaleY);

    CEGUIManipulator::updateHandles();
}

bool LayoutManipulator::isAnyHandleSelected() const
{
    if (_lcHandle && _lcHandle->isSelected()) return true;
    return CEGUIManipulator::isAnyHandleSelected();
}

void LayoutManipulator::deselectAllHandles()
{
    if (_lcHandle) _lcHandle->setSelected(false);
    CEGUIManipulator::deselectAllHandles();
}

bool LayoutManipulator::dropChildren(QGraphicsSceneDragDropEvent* event, size_t indexInParent) const
{
    // Takes care of creating new widgets when user drops the right mime type here
    // (dragging from the CreateWidgetDockWidget)
    auto bytes = event->mimeData()->data("application/x-ceed-widget-type");
    if (bytes.size() > 0)
    {
        if (canAcceptChildren(1, true))
            _visualMode.getEditor().getUndoStack()->push(
                        new LayoutCreateCommand(_visualMode, getWidgetPath(), bytes.data(), event->scenePos(), indexInParent));
        return true;
    }

    // Drop existing widgets into this widget as children with Ctrl+Drag or from widget tree
    bytes = event->mimeData()->data("application/x-ceed-widget-paths");
    if (bytes.size() > 0)
    {
        QStringList widgetPaths;
        QDataStream stream(&bytes, QIODevice::ReadOnly);
        while (!stream.atEnd())
        {
            QString name;
            stream >> name;
            widgetPaths.append(name);
        }

        CEGUIUtils::removeNestedPaths(widgetPaths);

        if (event->dropAction() == Qt::MoveAction)
        {
            // Remember desired offsets before reparenting
            std::map<LayoutManipulator*, QPointF> offsetByParent;
            std::map<CEGUI::Window*, CEGUI::UVector2> newPositions;

            // Layout containers align their children, no need to move them
            const bool isLC = isLayoutContainer();

            if (!isLC)
            {
                // - Dragged item is always the first (only for visual mode dragging for now, not from the tree)
                // - The first dragged item from each parent is dropped at a cursor position, with a small cascade offset
                // - All subsequent items from the same parent keep relative position to the first item
                const auto dropOffset = event->scenePos() - scenePos();

                for (const QString& widgetPath : qAsConst(widgetPaths))
                {
                    auto manipulator = _visualMode.getScene()->getManipulatorByPath(widgetPath);
                    if (!manipulator) continue;

                    auto oldParentManipulator = dynamic_cast<LayoutManipulator*>(manipulator->parentItem());
                    if (!oldParentManipulator) continue;

                    auto it = offsetByParent.find(oldParentManipulator);
                    if (it == offsetByParent.cend())
                    {
                        const qreal cascadeOffset = offsetByParent.size() * 10.0;
                        const QPointF offset = dropOffset - manipulator->pos() + QPointF(cascadeOffset, cascadeOffset);
                        it = offsetByParent.emplace(oldParentManipulator, offset).first;
                    }

                    if (it->second.manhattanLength() > 0.0)
                    {
                        auto newPos = manipulator->getWidget()->getPosition();
                        newPos.d_x.d_offset += static_cast<float>(it->second.x());
                        newPos.d_y.d_offset += static_cast<float>(it->second.y());
                        newPositions.emplace(manipulator->getWidget(), newPos);
                    }
                }
            }

            // Reparent widgets to the new parent
            _visualMode.moveWidgetsInHierarchy(widgetPaths, this, indexInParent);

            if (!isLC)
            {
                // Then move widgets using the drop position with a separate command
                std::vector<LayoutMoveCommand::Record> records;
                for (const auto& pair : newPositions)
                {
                    if (pair.first->getPosition() == pair.second) continue;

                    LayoutMoveCommand::Record rec;
                    rec.path = CEGUIUtils::stringToQString(pair.first->getNamePath());
                    rec.oldPos = pair.first->getPosition();
                    rec.newPos = pair.second;
                    records.push_back(std::move(rec));
                }
                if (!records.empty())
                    _visualMode.getEditor().getUndoStack()->push(new LayoutMoveCommand(_visualMode, std::move(records)));
            }

            return true;
        }
    }

    return false;
}

void LayoutManipulator::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
{
    if (event->mimeData()->hasFormat("application/x-ceed-widget-type") ||
        event->mimeData()->hasFormat("application/x-ceed-widget-paths"))
    {
        QPen pen(Qt::PenStyle::DashLine);
        pen.setColor(Qt::white);
        pen.setWidth(2);
        pen.setCosmetic(true);
        setPen(pen);

        event->acceptProposedAction();
        _visualMode.getScene()->onManipulatorDragEnter(this);
    }
    else
    {
        event->ignore();
        _visualMode.getScene()->onManipulatorDragEnter(nullptr);
    }
}

void LayoutManipulator::dragLeaveEvent(QGraphicsSceneDragDropEvent* event)
{
    _visualMode.getScene()->onManipulatorDragLeave(this);
    resetPen();
    CEGUIManipulator::dragLeaveEvent(event);
}

void LayoutManipulator::dropEvent(QGraphicsSceneDragDropEvent* event)
{
    if (dropChildren(event))
        event->acceptProposedAction();
    else
        event->ignore();
}

void LayoutManipulator::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    // Ctrl+Drag to reparent selected widgets
    if (event->modifiers() & Qt::ControlModifier)
    {
        event->accept();

        if (QLineF(event->screenPos(), event->buttonDownScreenPos(Qt::LeftButton)).length() >= QApplication::startDragDistance())
        {
            setCursor(Qt::ClosedHandCursor);

            std::set<LayoutManipulator*> selectedWidgets;

            /* UX: not sure what is better, move all selection or only a Ctrl-dragged item
            if (!isSelected() && !isAnyHandleSelected())
            {
                _visualMode.getScene()->clearSelection();
            }
            else
            */
            {
                _visualMode.getScene()->collectSelectedWidgets(selectedWidgets);
                removeNestedManipulators(selectedWidgets);
                selectedWidgets.erase(this); // Will add to stream manually
            }

            QByteArray bytes;
            QDataStream stream(&bytes, QIODevice::WriteOnly);

            // Add dragged widget first, then the rest of selection
            stream << getWidgetPath();
            for (LayoutManipulator* manipulator : selectedWidgets)
                stream << manipulator->getWidgetPath();

            QDrag* drag = new QDrag(event->widget());
            QMimeData* mime = new QMimeData();
            drag->setMimeData(mime);
            mime->setData("application/x-ceed-widget-paths", bytes);

            /*
                QPixmap pixmap(34, 34);
                pixmap.fill(Qt::white);

                QPainter painter(&pixmap);
                painter.translate(15, 15);
                painter.setRenderHint(QPainter::Antialiasing);
                paint(&painter, 0, 0);
                painter.end();

                pixmap.setMask(pixmap.createHeuristicMask());

                drag->setPixmap(pixmap);
                drag->setHotSpot(QPoint(15, 20));
            */

            drag->exec();
            setCursor(Qt::OpenHandCursor);
        }

        return;
    }

    CEGUIManipulator::mouseMoveEvent(event);
}

QVariant LayoutManipulator::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemSelectedHasChanged)
    {
        if (_lcHandle) _lcHandle->updateLook();
        else
        {
            // If we are inside a layout container, its handle must be on top of
            // all other LCs in the same hierarchy for better user interaction
            auto parent = parentItem();
            while (parent)
            {
                if (auto parentManipulator = dynamic_cast<LayoutManipulator*>(parent))
                {
                    if (parentManipulator->isLayoutContainer())
                    {
                        parentManipulator->_lcHandle->updateLook();
                        break;
                    }
                }
                parent = parent->parentItem();
            }
        }
    }
    else if (change == ItemSceneChange)
    {
        _visualMode.getScene()->onManipulatorRemoved(this);
    }

    return CEGUIManipulator::itemChange(change, value);
}

void LayoutManipulator::onPropertyChanged(const QtnPropertyBase* property, CEGUI::Property* ceguiProperty)
{
    QString value;
    if (!property->toStr(value)) return;

    const auto& propertyName = ceguiProperty->getName();

    // Special case: when we edit the name, widget path changes and LayoutPropertyEditCommand
    // will fail to find the widget. Use LayoutRenameCommand here. Also it is more consistent.
    if (propertyName == "Name")
    {
        QString newName = value;
        if (!renameWidget(newName))
            newName = getWidgetName();

        // Set actual name back to the property
        if (newName != value)
            updatePropertiesFromWidget({"Name"});

        return;
    }

    const bool isGridWidth = (propertyName == "GridWidth");
    if (isGridWidth || propertyName == "GridHeight")
    {
        auto glc = dynamic_cast<CEGUI::GridLayoutContainer*>(_widget);
        if (glc)
        {
            size_t minWidth, minHeight;
            glc->getMinimalSizeInCells(minWidth, minHeight);
            const auto newCells = static_cast<size_t>(property->valueAsVariant().toULongLong());
            const auto minCells = isGridWidth ? minWidth : minHeight;

            if (newCells < minCells)
            {
                // Set actual value back to the property
                updatePropertiesFromWidget({property->name()});

                // Synchronous message box leads to a crash here
                QTimer::singleShot(0, [isGridWidth, minCells]()
                {
                    QMessageBox::warning(nullptr, "Can't reduce grid size",
                                         QString("Grid %1 can't be set below %2 because of grid contents.\n"
                                                 "Delete excess children manually and try again.")
                                         .arg(isGridWidth ? "width" : "height").arg(minCells));
                });

                return;
            }
        }
    }

    std::vector<LayoutPropertyEditCommand::Record> records;

    LayoutPropertyEditCommand::Record rec;
    rec.path = getWidgetPath();
    rec.oldValue = ceguiProperty->get(_widget);
    rec.newValue = CEGUIUtils::qStringToString(value);
    records.push_back(std::move(rec));

    // Handle multiproperty merge in a command itself
    const size_t groupId = _visualMode.getScene()->getMultiSelectionChangeId();
    auto cmd = new LayoutPropertyEditCommand(_visualMode, std::move(records), property->name(), groupId);

    auto undoStack = _visualMode.getEditor().getUndoStack();
    undoStack->push(cmd);

    // TODO: we could avoid that if CEGUI allowed us to check validity of a property value without setting it
    const bool notMerged = undoStack->command(undoStack->count() - 1) == cmd;
    if (notMerged && cmd->isValueInvalid())
    {
        cmd->setObsolete(true);
        _visualMode.getEditor().getUndoStack()->undo();
    }
}

void LayoutManipulator::onWidgetNameChanged()
{
    CEGUIManipulator::onWidgetNameChanged();
    if (_treeItem) _treeItem->refreshPathData();
    if (_lcHandle) _lcHandle->updateTooltip();
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

#include "src/ui/layout/LayoutScene.h"
#include "src/ui/layout/LayoutManipulator.h"
#include "src/ui/layout/WidgetHierarchyDockWidget.h"
#include "src/ui/layout/WidgetHierarchyItem.h"
#include "src/ui/ResizingHandle.h"
#include "src/editors/layout/LayoutVisualMode.h"
#include "src/editors/layout/LayoutUndoCommands.h"
#include "src/ui/GuideLine.h"
#include <CEGUI/CoordConverter.h>
#include <CEGUI/GUIContext.h>
#include <CEGUI/widgets/SequentialLayoutContainer.h>
#include "qgraphicssceneevent.h"
#include "qevent.h"
#include "qmimedata.h"
#include "qtreeview.h"
#include "qstandarditemmodel.h"
#include <set>

// For properties (may be incapsulated somewhere):
#include "src/ui/MainWindow.h"
#include "src/Application.h"
#include "PropertyWidget/PropertyWidget.h"

LayoutScene::LayoutScene(LayoutVisualMode& visualMode)
    : _visualMode(visualMode)
{
    connect(this, &LayoutScene::selectionChanged, this, &LayoutScene::slot_selectionChanged);
}

void LayoutScene::updateFromWidgets()
{
    if (rootManipulator) rootManipulator->updateFromWidget();
}

// Overridden to keep the manipulators in sync
void LayoutScene::setCEGUIDisplaySize(float width, float height)
{
    CEGUIGraphicsScene::setCEGUIDisplaySize(width, height);
    updateFromWidgets();
}

void LayoutScene::setRootWidgetManipulator(LayoutManipulator* manipulator)
{
    clear();

    rootManipulator = manipulator;

    if (rootManipulator)
    {
        ceguiContext->setRootWindow(rootManipulator->getWidget());

        // Root manipulator changed, perform a full update
        // NB: widget must be already set to a CEGUI context for area calculation
        rootManipulator->updateFromWidget(true);
        addItem(rootManipulator);

        //!!!DBG TMP!
        GuideLine* _anchorMinX = new GuideLine(false, nullptr, 2, Qt::PenStyle::DashLine, Qt::white, Qt::cyan);
        addItem(_anchorMinX);
        _anchorMinX->setPos(150.0, 0.0);
        /*
            // Update anchors
            if (_showOutline && scene())
            {
                _anchors->setAnchors(_widget->getPosition().d_x.d_scale,
                                     _widget->getPosition().d_y.d_scale,
                                     _widget->getPosition().d_x.d_scale + _widget->getSize().d_width.d_scale,
                                     _widget->getPosition().d_y.d_scale + _widget->getSize().d_height.d_scale);
                _anchors->setVisible(true);
            }
        */
    }
    else
        ceguiContext->setRootWindow(nullptr);
}

LayoutManipulator* LayoutScene::getManipulatorByPath(const QString& widgetPath) const
{
    if (!rootManipulator) return nullptr;

    auto sepPos = widgetPath.indexOf('/');
    if (sepPos < 0)
    {
        assert(widgetPath == rootManipulator->getWidgetName());
        return rootManipulator;
    }
    else
    {
        assert(widgetPath.leftRef(sepPos) == rootManipulator->getWidgetName());
        return dynamic_cast<LayoutManipulator*>(rootManipulator->getManipulatorByPath(widgetPath.mid(sepPos + 1)));
    }
}

void LayoutScene::normalizePositionOfSelectedWidgets()
{
    std::vector<LayoutMoveCommand::Record> records;
    bool toRelative = false;

    for (QGraphicsItem* item : selectedItems())
    {
        auto manipulator = dynamic_cast<LayoutManipulator*>(item);
        if (!manipulator) continue;

        const auto pos = manipulator->getWidget()->getPosition();

        LayoutMoveCommand::Record rec;
        rec.path = manipulator->getWidgetPath();
        rec.oldPos = pos;
        records.push_back(std::move(rec));

        // For absolute-relative cycling
        if (!toRelative && (pos.d_x.d_offset != 0.f || pos.d_y.d_offset != 0.f))
            toRelative = true;
    }

    if (records.empty()) return;

    for (auto& rec : records)
    {
        const LayoutManipulator* manipulator = getManipulatorByPath(rec.path);
        const auto baseSize = manipulator->getBaseSize();

        if (toRelative) // command id was base + 12
        {
            rec.newPos.d_x.d_scale = (rec.oldPos.d_x.d_offset + rec.oldPos.d_x.d_scale * baseSize.d_width) / baseSize.d_width;
            rec.newPos.d_x.d_offset = 0.f;
            rec.newPos.d_y.d_scale = (rec.oldPos.d_y.d_offset + rec.oldPos.d_y.d_scale * baseSize.d_height) / baseSize.d_height;
            rec.newPos.d_y.d_offset = 0.f;
        }
        else // command id was base + 13
        {
            rec.newPos.d_x.d_scale = 0.f;
            rec.newPos.d_x.d_offset = rec.oldPos.d_x.d_offset + rec.oldPos.d_x.d_scale * baseSize.d_width;
            rec.newPos.d_y.d_scale = 0.f;
            rec.newPos.d_y.d_offset = rec.oldPos.d_y.d_offset + rec.oldPos.d_y.d_scale * baseSize.d_height;
        }
    }

    _visualMode.getEditor().getUndoStack()->push(new LayoutMoveCommand(_visualMode, std::move(records)));
}

void LayoutScene::normalizeSizeOfSelectedWidgets()
{
    std::vector<LayoutResizeCommand::Record> records;
    bool toRelative = false;

    for (QGraphicsItem* item : selectedItems())
    {
        auto manipulator = dynamic_cast<LayoutManipulator*>(item);
        if (!manipulator) continue;

        const auto size = manipulator->getWidget()->getSize();

        LayoutResizeCommand::Record rec;
        rec.path = manipulator->getWidgetPath();
        rec.oldPos = manipulator->getWidget()->getPosition();
        rec.oldSize = size;
        rec.newPos = rec.oldPos;
        records.push_back(std::move(rec));

        // For absolute-relative cycling
        if (!toRelative && (size.d_width.d_offset != 0.f || size.d_height.d_offset != 0.f))
            toRelative = true;
    }

    if (records.empty()) return;

    for (auto& rec : records)
    {
        const LayoutManipulator* manipulator = getManipulatorByPath(rec.path);
        const auto pixelSize = manipulator->getWidget()->getPixelSize();

        if (toRelative) // command id was base + 10
        {
            const auto baseSize = manipulator->getBaseSize();
            rec.newSize.d_width.d_scale = pixelSize.d_width / baseSize.d_width;
            rec.newSize.d_width.d_offset = 0.f;
            rec.newSize.d_height.d_scale = pixelSize.d_height / baseSize.d_height;
            rec.newSize.d_height.d_offset = 0.f;
        }
        else // command id was base + 11
        {
            rec.newSize.d_width.d_scale = 0.f;
            rec.newSize.d_width.d_offset = pixelSize.d_width;
            rec.newSize.d_height.d_scale = 0.f;
            rec.newSize.d_height.d_offset = pixelSize.d_height;
        }
    }

    _visualMode.getEditor().getUndoStack()->push(new LayoutResizeCommand(_visualMode, std::move(records)));
}

// command id was base + 15
void LayoutScene::roundPositionOfSelectedWidgets()
{
    std::vector<LayoutMoveCommand::Record> records;
    for (QGraphicsItem* item : selectedItems())
    {
        if (auto manipulator = dynamic_cast<LayoutManipulator*>(item))
        {
            LayoutMoveCommand::Record rec;
            rec.path = manipulator->getWidgetPath();
            rec.oldPos = manipulator->getWidget()->getPosition();
            rec.newPos = rec.oldPos;
            rec.newPos.d_x.d_offset = CEGUI::CoordConverter::alignToPixels(rec.oldPos.d_x.d_offset);
            rec.newPos.d_y.d_offset = CEGUI::CoordConverter::alignToPixels(rec.oldPos.d_y.d_offset);
            records.push_back(std::move(rec));
        }
    }

    _visualMode.getEditor().getUndoStack()->push(new LayoutMoveCommand(_visualMode, std::move(records)));
}

void LayoutScene::roundSizeOfSelectedWidgets()
{
    std::vector<LayoutResizeCommand::Record> records;
    for (QGraphicsItem* item : selectedItems())
    {
        if (auto manipulator = dynamic_cast<LayoutManipulator*>(item))
        {
            LayoutResizeCommand::Record rec;
            rec.path = manipulator->getWidgetPath();
            rec.oldPos = manipulator->getWidget()->getPosition();
            rec.newPos = rec.oldPos;
            rec.oldSize = manipulator->getWidget()->getSize();
            rec.newSize = rec.oldSize;
            rec.newSize.d_width.d_offset = CEGUI::CoordConverter::alignToPixels(rec.oldSize.d_width.d_offset);
            rec.newSize.d_height.d_offset = CEGUI::CoordConverter::alignToPixels(rec.oldSize.d_height.d_offset);
            records.push_back(std::move(rec));
        }
    }

    _visualMode.getEditor().getUndoStack()->push(new LayoutResizeCommand(_visualMode, std::move(records)));
}

void LayoutScene::alignSelectionHorizontally(CEGUI::HorizontalAlignment alignment)
{
    std::vector<LayoutHorizontalAlignCommand::Record> records;
    for (QGraphicsItem* item : selectedItems())
    {
        if (auto manipulator = dynamic_cast<LayoutManipulator*>(item))
        {
            LayoutHorizontalAlignCommand::Record rec;
            rec.path = manipulator->getWidgetPath();
            rec.oldAlignment = manipulator->getWidget()->getHorizontalAlignment();
            records.push_back(std::move(rec));
        }
    }

    _visualMode.getEditor().getUndoStack()->push(new LayoutHorizontalAlignCommand(_visualMode, std::move(records), alignment));
}

void LayoutScene::alignSelectionVertically(CEGUI::VerticalAlignment alignment)
{
    std::vector<LayoutVerticalAlignCommand::Record> records;
    for (QGraphicsItem* item : selectedItems())
    {
        if (auto manipulator = dynamic_cast<LayoutManipulator*>(item))
        {
            LayoutVerticalAlignCommand::Record rec;
            rec.path = manipulator->getWidgetPath();
            rec.oldAlignment = manipulator->getWidget()->getVerticalAlignment();
            records.push_back(std::move(rec));
        }
    }

    _visualMode.getEditor().getUndoStack()->push(new LayoutVerticalAlignCommand(_visualMode, std::move(records), alignment));
}

void LayoutScene::moveSelectedWidgetsInParentWidgetLists(int delta)
{
    QStringList paths;
    for (QGraphicsItem* item : selectedItems())
    {
        auto manipulator = dynamic_cast<LayoutManipulator*>(item);
        if (!manipulator) continue;

        auto parentManipulator = dynamic_cast<LayoutManipulator*>(item->parentItem());
        if (!parentManipulator) continue;

        auto container = dynamic_cast<CEGUI::SequentialLayoutContainer*>(parentManipulator->getWidget());
        if (!container) continue;

        int potentialPos = static_cast<int>(container->getPositionOfChild(manipulator->getWidget())) + delta;
        if (potentialPos < 0 || potentialPos >= static_cast<int>(container->getChildCount())) continue;

        paths.append(manipulator->getWidgetPath());
    }

    // TODO: We currently only support moving one widget at a time.
    //       Fixing this involves sorting the widgets by their position in
    //       the parent widget and then either working from the "right" side
    //       if delta > 0 or from the left side if delta < 0.
    if (paths.size() == 1)
        _visualMode.getEditor().getUndoStack()->push(new MoveInParentWidgetListCommand(_visualMode, std::move(paths), delta));
}

bool LayoutScene::deleteSelectedWidgets()
{
    QStringList widgetPaths;
    for (auto& item : selectedItems())
    {
        auto manip = dynamic_cast<LayoutManipulator*>(item);
        if (manip) widgetPaths.push_back(manip->getWidgetPath());
    }

    if (!widgetPaths.isEmpty())
        _visualMode.getEditor().getUndoStack()->push(new LayoutDeleteCommand(_visualMode, std::move(widgetPaths)));

    return true;
}

static void ensureParentIsExpanded(QTreeView* view, QStandardItem* treeItem)
{
    view->expand(treeItem->index());
    if (treeItem->parent())
        ensureParentIsExpanded(view, treeItem->parent());
}

void LayoutScene::slot_selectionChanged()
{
    std::set<QtnPropertySet*> selectedWidgets;

    auto selection = selectedItems();
    for (QGraphicsItem* item : selection)
    {
        if (auto manipulator = dynamic_cast<LayoutManipulator*>(item))
        {
            selectedWidgets.insert(manipulator->getPropertySet());
        }
        else if (dynamic_cast<ResizingHandle*>(item))
        {
            if (auto manipulator = dynamic_cast<LayoutManipulator*>(item->parentItem()))
                selectedWidgets.insert(manipulator->getPropertySet());
        }
    }

    // TODO: to method (whose?)
    QtnPropertySet* propertySet = nullptr;
    for (QtnPropertySet* set : selectedWidgets)
    {
        // TODO: create multi property set
        //!!!pass vector of sources & combine inside an entry point method!
        propertySet = set;
    }
    auto mainWindow = qobject_cast<Application*>(qApp)->getMainWindow();
    auto propertyWidget = static_cast<QtnPropertyWidget*>(mainWindow->getPropertyDockWidget()->widget());
    propertyWidget->setPropertySet(propertySet);

    // We always sync the properties dock widget, we only ignore the hierarchy synchro if told so
    if (!_ignoreSelectionChanges)
    {
        _visualMode.getHierarchyDockWidget()->ignoreSelectionChanges(true);

        _visualMode.getHierarchyDockWidget()->getTreeView()->clearSelection();

        auto treeView = _visualMode.getHierarchyDockWidget()->getTreeView();

        QStandardItem* lastTreeItem = nullptr;
        for (QGraphicsItem* item : selection)
        {
            auto manip = dynamic_cast<LayoutManipulator*>(item);
            if (manip && manip->getTreeItem())
            {
                treeView->selectionModel()->select(manip->getTreeItem()->index(), QItemSelectionModel::Select);
                ensureParentIsExpanded(treeView, manip->getTreeItem());
                lastTreeItem = manip->getTreeItem();
            }
        }

        if (lastTreeItem) treeView->scrollTo(lastTreeItem->index());

        _visualMode.getHierarchyDockWidget()->ignoreSelectionChanges(false);
    }
}

void LayoutScene::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
{
    // If the root manipulator is in place the QGraphicsScene machinery will take care of drag n drop
    // the graphics items (manipulators in fact) have that implemented already
    if (rootManipulator)
        CEGUIGraphicsScene::dragEnterEvent(event);
    else
    {
        // Otherwise we should accept a new root widget to the empty layout if it's a new widget
        if (event->mimeData()->hasFormat("application/x-ceed-widget-type"))
            event->acceptProposedAction();
    }
}

void LayoutScene::dragLeaveEvent(QGraphicsSceneDragDropEvent* event)
{
    // If the root manipulator is in place the QGraphicsScene machinery will take care of drag n drop
    // the graphics items (manipulators in fact) have that implemented already
    if (rootManipulator)
        CEGUIGraphicsScene::dragEnterEvent(event);
}

void LayoutScene::dragMoveEvent(QGraphicsSceneDragDropEvent* event)
{
    // If the root manipulator is in place the QGraphicsScene machinery will take care of drag n drop
    // the graphics items (manipulators in fact) have that implemented already
    if (rootManipulator)
        CEGUIGraphicsScene::dragMoveEvent(event);
    else
    {
        // Otherwise we should accept a new root widget to the empty layout if it's a new widget
        if (event->mimeData()->hasFormat("application/x-ceed-widget-type"))
            event->acceptProposedAction();
    }
}

void LayoutScene::dropEvent(QGraphicsSceneDragDropEvent* event)
{
    // If the root manipulator is in place the QGraphicsScene machinery will take care of drag n drop
    // the graphics items (manipulators in fact) have that implemented already
    if (rootManipulator)
        CEGUIGraphicsScene::dropEvent(event);
    else
    {
        auto data = event->mimeData()->data("application/x-ceed-widget-type");
        if (data.size() > 0)
        {
            QString widgetType = data.data();
            int sepPos = widgetType.lastIndexOf('/');
            QString widgetName = (sepPos < 0) ? widgetType : widgetType.mid(sepPos + 1);
            _visualMode.getEditor().getUndoStack()->push(new LayoutCreateCommand(_visualMode, "", widgetType, widgetName));

            event->acceptProposedAction();
        }
        else
        {
            event->ignore();
        }
    }
}

void LayoutScene::keyReleaseEvent(QKeyEvent* event)
{
    bool handled = false;

    if (event->key() == Qt::Key_Delete)
    {
        handled = deleteSelectedWidgets();
    }

    if (handled)
        event->accept();
    else
        CEGUIGraphicsScene::keyReleaseEvent(event);
}

void LayoutScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    CEGUIGraphicsScene::mouseReleaseEvent(event);

    // We have to "expand" the items, adding parents of resizing handles instead of the handles themselves
    std::vector<LayoutManipulator*> selection;
    for (QGraphicsItem* selectedItem : selectedItems())
    {
        if (auto manipulator = dynamic_cast<LayoutManipulator*>(selectedItem))
        {
            selection.push_back(manipulator);
            continue;
        }

        if (auto handle = dynamic_cast<ResizingHandle*>(selectedItem))
        {
            if (auto manipulator = dynamic_cast<LayoutManipulator*>(handle->parentItem()))
            {
                selection.push_back(manipulator);
                continue;
            }
        }
    }

    std::vector<LayoutMoveCommand::Record> move;
    std::vector<LayoutResizeCommand::Record> resize;

    for (LayoutManipulator* item : selection)
    {
        if (item->isMoveStarted())
        {
            LayoutMoveCommand::Record rec;
            rec.path = item->getWidgetPath();
            rec.oldPos = item->getMoveStartPosition();
            rec.newPos = item->getWidget()->getPosition();
            move.push_back(std::move(rec));

            item->resetMove();
        }

        if (item->isResizeStarted())
        {
            LayoutResizeCommand::Record rec;
            rec.path = item->getWidgetPath();
            rec.oldPos = item->getResizeStartPosition();
            rec.newPos = item->getWidget()->getPosition();
            rec.oldSize = item->getResizeStartSize();
            rec.newSize = item->getWidget()->getSize();
            resize.push_back(std::move(rec));

            item->resetResize();
        }
    }

    if (!move.empty())
        _visualMode.getEditor().getUndoStack()->push(new LayoutMoveCommand(_visualMode, std::move(move)));

    if (!resize.empty())
        _visualMode.getEditor().getUndoStack()->push(new LayoutResizeCommand(_visualMode, std::move(resize)));
}

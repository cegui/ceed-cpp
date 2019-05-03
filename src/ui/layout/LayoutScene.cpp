#include "src/ui/layout/LayoutScene.h"
#include "src/ui/layout/LayoutManipulator.h"
#include "src/ui/layout/WidgetHierarchyDockWidget.h"
#include "src/ui/layout/WidgetHierarchyItem.h"
#include "src/ui/ResizingHandle.h"
#include "src/editors/layout/LayoutVisualMode.h"
#include "src/editors/layout/LayoutUndoCommands.h"
#include <CEGUI/GUIContext.h>
#include "qgraphicssceneevent.h"
#include "qevent.h"
#include "qmimedata.h"
#include "qtreeview.h"
#include "qstandarditemmodel.h"
#include <set>

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
        // Root manipulator changed, perform a full update
        rootManipulator->updateFromWidget(true);
        addItem(rootManipulator);

        ceguiContext->setRootWindow(rootManipulator->getWidget());
    }
    else
        ceguiContext->setRootWindow(nullptr);
}

LayoutManipulator* LayoutScene::getManipulatorByPath(const QString& widgetPath) const
{
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
/*
        widgetPaths = []
        oldPositions = {}

        # if there will be no non-zero offsets, we will normalise to absolute
        undoCommand = undo.NormalisePositionToAbsoluteCommand

        selection = self.selectedItems()
        for item in selection:
            if isinstance(item, widgethelpers.Manipulator):
                widgetPath = item.widget.getNamePath()

                widgetPaths.append(widgetPath)
                oldPositions[widgetPath] = item.widget.getPosition()

                # if we find any non-zero offset, normalise to relative
                if (item.widget.getPosition().d_x.d_offset != 0) or (item.widget.getPosition().d_y.d_offset != 0):
                    undoCommand = undo.NormalisePositionToRelativeCommand

        if len(widgetPaths) > 0:
            cmd = undoCommand(self.visual, widgetPaths, oldPositions)
            self.visual.tabbedEditor.undoStack.push(cmd)
*/
}

void LayoutScene::normalizeSizeOfSelectedWidgets()
{
/*
        widgetPaths = []
        oldPositions = {}
        oldSizes = {}

        # if there will be no non-zero offsets, we will normalise to absolute
        undoCommand = undo.NormaliseSizeToAbsoluteCommand

        selection = self.selectedItems()
        for item in selection:
            if isinstance(item, widgethelpers.Manipulator):
                widgetPath = item.widget.getNamePath()

                widgetPaths.append(widgetPath)
                oldPositions[widgetPath] = item.widget.getPosition()
                oldSizes[widgetPath] = item.widget.getSize()

                # if we find any non-zero offset, normalise to relative
                if (item.widget.getSize().d_width.d_offset != 0) or (item.widget.getSize().d_height.d_offset != 0):
                    undoCommand = undo.NormaliseSizeToRelativeCommand

        if len(widgetPaths) > 0:
            cmd = undoCommand(self.visual, widgetPaths, oldPositions, oldSizes)
            self.visual.tabbedEditor.undoStack.push(cmd)
*/
}

void LayoutScene::roundPositionOfSelectedWidgets()
{
/*
        widgetPaths = []
        oldPositions = {}

        selection = self.selectedItems()
        for item in selection:
            if isinstance(item, widgethelpers.Manipulator):
                widgetPath = item.widget.getNamePath()

                widgetPaths.append(widgetPath)
                oldPositions[widgetPath] = item.widget.getPosition()

        if len(widgetPaths) > 0:
            cmd = undo.RoundPositionCommand(self.visual, widgetPaths, oldPositions)
            self.visual.tabbedEditor.undoStack.push(cmd)
*/
}

void LayoutScene::roundSizeOfSelectedWidgets()
{
/*
        widgetPaths = []
        oldPositions = {}
        oldSizes = {}

        selection = self.selectedItems()
        for item in selection:
            if isinstance(item, widgethelpers.Manipulator):
                widgetPath = item.widget.getNamePath()

                widgetPaths.append(widgetPath)
                oldPositions[widgetPath] = item.widget.getPosition()
                oldSizes[widgetPath] = item.widget.getSize()

        if len(widgetPaths) > 0:
            cmd = undo.RoundSizeCommand(self.visual, widgetPaths, oldPositions, oldSizes)
            self.visual.tabbedEditor.undoStack.push(cmd)
*/
}

void LayoutScene::alignSelectionHorizontally(CEGUI::HorizontalAlignment alignment)
{
/*
        widgetPaths = []
        oldAlignments = {}

        selection = self.selectedItems()
        for item in selection:
            if isinstance(item, widgethelpers.Manipulator):
                widgetPath = item.widget.getNamePath()
                widgetPaths.append(widgetPath)
                oldAlignments[widgetPath] = item.widget.getHorizontalAlignment()

        if len(widgetPaths) > 0:
            cmd = undo.HorizontalAlignCommand(self.visual, widgetPaths, oldAlignments, alignment)
            self.visual.tabbedEditor.undoStack.push(cmd)
*/
}

void LayoutScene::alignSelectionVertically(CEGUI::VerticalAlignment alignment)
{
/*
        widgetPaths = []
        oldAlignments = {}

        selection = self.selectedItems()
        for item in selection:
            if isinstance(item, widgethelpers.Manipulator):
                widgetPath = item.widget.getNamePath()
                widgetPaths.append(widgetPath)
                oldAlignments[widgetPath] = item.widget.getVerticalAlignment()

        if len(widgetPaths) > 0:
            cmd = undo.VerticalAlignCommand(self.visual, widgetPaths, oldAlignments, alignment)
            self.visual.tabbedEditor.undoStack.push(cmd)
*/
}

void LayoutScene::moveSelectedWidgetsInParentWidgetLists(int delta)
{
/*
        widgetPaths = []

        selection = self.selectedItems()
        for item in selection:
            if not isinstance(item, widgethelpers.Manipulator):
                continue

            if not isinstance(item.parentItem(), widgethelpers.Manipulator):
                continue

            if not isinstance(item.parentItem().widget, PyCEGUI.SequentialLayoutContainer):
                continue

            potentialPosition = item.parentItem().widget.getPositionOfChild(item.widget) + delta
            if potentialPosition < 0 or potentialPosition > item.parentItem().widget.getChildCount() - 1:
                continue

            widgetPath = item.widget.getNamePath()
            widgetPaths.append(widgetPath)

        # TODO: We currently only support moving one widget at a time.
        #       Fixing this involves sorting the widgets by their position in
        #       the parent widget and then either working from the "right" side
        #       if delta > 0 or from the left side if delta < 0.
        if len(widgetPaths) == 1:
            cmd = undo.MoveInParentWidgetListCommand(self.visual, widgetPaths, delta)
            self.visual.tabbedEditor.undoStack.push(cmd)
*/
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
    {
/*
            cmd = undo.DeleteCommand(self.visual, widgetPaths)
            self.visual.tabbedEditor.undoStack.push(cmd)
*/
    }

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
    std::set<CEGUI::Window*> selectedWidgets;

    auto selection = selectedItems();
    for (QGraphicsItem* item : selection)
    {
        if (auto manipulator = dynamic_cast<LayoutManipulator*>(item))
        {
            selectedWidgets.insert(manipulator->getWidget());
        }
        else if (dynamic_cast<ResizingHandle*>(item))
        {
            if (auto manipulator = dynamic_cast<LayoutManipulator*>(item->parentItem()))
                selectedWidgets.insert(manipulator->getWidget());
        }
    }
/*
        auto mainWindow = qobject_cast<Application*>(qApp)->getMainWindow();
        mainWindow.propertiesDockWidget.inspector.setSource(sets)
*/

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
            /*
                    cmd = undo.CreateCommand(self.visual, "", widgetType, widgetType.rsplit("/", 1)[-1])
                    self.visual.tabbedEditor.undoStack.push(cmd)
            */
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
    //std::vector<LayoutResizeCommand::Record> resize;

    for (LayoutManipulator* manipulator : selection)
    {
    /*
        if item.preMovePos is not None:
            widgetPath = item.widget.getNamePath()
            movedWidgetPaths.append(widgetPath)
            movedOldPositions[widgetPath] = item.preMovePos
            movedNewPositions[widgetPath] = item.widget.getPosition()

            # it won't be needed anymore so we use this to mark we picked this item up
            item.preMovePos = None

        if item.preResizePos is not None and item.preResizeSize is not None:
            widgetPath = item.widget.getNamePath()
            resizedWidgetPaths.append(widgetPath)
            resizedOldPositions[widgetPath] = item.preResizePos
            resizedOldSizes[widgetPath] = item.preResizeSize
            resizedNewPositions[widgetPath] = item.widget.getPosition()
            resizedNewSizes[widgetPath] = item.widget.getSize()

            # it won't be needed anymore so we use this to mark we picked this item up
            item.preResizePos = None
            item.preResizeSize = None
    */
    }

    if (!move.empty())
        _visualMode.getEditor().getUndoStack()->push(new LayoutMoveCommand(_visualMode, std::move(move)));
/*

        if len(resizedWidgetPaths) > 0:
            cmd = undo.ResizeCommand(self.visual, resizedWidgetPaths, resizedOldPositions, resizedOldSizes, resizedNewPositions, resizedNewSizes)
            self.visual.tabbedEditor.undoStack.push(cmd)
*/
}

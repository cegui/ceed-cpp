#include "src/ui/layout/WidgetHierarchyTreeView.h"
#include "src/ui/layout/WidgetHierarchyItem.h"
#include "src/ui/layout/WidgetHierarchyDockWidget.h"
#include "src/ui/layout/LayoutScene.h"
#include "src/ui/layout/LayoutManipulator.h"
#include "src/editors/layout/LayoutVisualMode.h"
#include "qmenu.h"
#include "qevent.h"
#include "qstandarditemmodel.h"

WidgetHierarchyTreeView::WidgetHierarchyTreeView(QWidget* parent)
    : QTreeView(parent)
{
}

void WidgetHierarchyTreeView::setupContextMenu()
{
    setContextMenuPolicy(Qt::DefaultContextMenu);

    contextMenu = new QMenu(this);
/*

        self.renameAction = action.getAction("layout/rename")
        self.contextMenu.addAction(self.renameAction)

        self.contextMenu.addSeparator()

        self.lockAction = action.getAction("layout/lock_widget")
        self.contextMenu.addAction(self.lockAction)
        self.unlockAction = action.getAction("layout/unlock_widget")
        self.contextMenu.addAction(self.unlockAction)
        self.recursivelyLockAction = action.getAction("layout/recursively_lock_widget")
        self.contextMenu.addAction(self.recursivelyLockAction)
        self.recursivelyUnlockAction = action.getAction("layout/recursively_unlock_widget")
        self.contextMenu.addAction(self.recursivelyUnlockAction)

        self.contextMenu.addSeparator()

        self.cutAction = action.getAction("all_editors/cut")
        self.contextMenu.addAction(self.cutAction)
        self.copyAction = action.getAction("all_editors/copy")
        self.contextMenu.addAction(self.copyAction)
        self.pasteAction = action.getAction("all_editors/paste")
        self.contextMenu.addAction(self.pasteAction)
        self.deleteAction = action.getAction("all_editors/delete")
        self.contextMenu.addAction(self.deleteAction)

        self.contextMenu.addSeparator()

        self.copyNamePathAction = action.getAction("layout/copy_widget_path")
        self.contextMenu.addAction(self.copyNamePathAction)
*/
}

// Synchronizes tree selection with scene selection
void WidgetHierarchyTreeView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    QTreeView::selectionChanged(selected, deselected);

    // We are running synchronization the other way, this prevents infinite loops and recursion
    WidgetHierarchyDockWidget* widget = dynamic_cast<WidgetHierarchyDockWidget*>(parentWidget());
    if (!widget || widget->isIgnoringSelectionChanges()) return;

    widget->ignoreSelectionChangesInScene(true);

    for (auto& index : selected.indexes())
    {
        auto item = static_cast<QStandardItemModel*>(model())->itemFromIndex(index);

        auto widgetItem = dynamic_cast<WidgetHierarchyItem*>(item);
        if (!widgetItem) continue;

        QString manipulatorPath = widgetItem->data(Qt::UserRole).toString();
        LayoutManipulator* manipulator = nullptr;
        if (!manipulatorPath.isEmpty())
            manipulator = widget->getVisualMode()->getScene()->getManipulatorByPath(manipulatorPath);

        if (manipulator) manipulator->setSelected(true);
    }

    for (auto& index : deselected.indexes())
    {
        auto item = static_cast<QStandardItemModel*>(model())->itemFromIndex(index);

        auto widgetItem = dynamic_cast<WidgetHierarchyItem*>(item);
        if (!widgetItem) continue;

        QString manipulatorPath = widgetItem->data(Qt::UserRole).toString();
        LayoutManipulator* manipulator = nullptr;
        if (!manipulatorPath.isEmpty())
            manipulator = widget->getVisualMode()->getScene()->getManipulatorByPath(manipulatorPath);

        if (manipulator) manipulator->setSelected(false);
    }

    widget->ignoreSelectionChangesInScene(false);
}

void WidgetHierarchyTreeView::contextMenuEvent(QContextMenuEvent* event)
{
    if (!contextMenu) return;

    auto selectedIndices = selectedIndexes();

    // TODO: since these actions enabled state depends on the selection,
    // move the enabling/disabling to a central "selection changed" handler.
    // The handler should be called on tab activations too because
    // activating a tab changes the selection, effectively.
    // We don't touch the cut/copy/paste actions because they're shared
    // among all editors and disabling them here would disable them
    // for the other editors too.
    const bool hasSelection = !selectedIndices.empty();
/*
        self.copyNamePathAction.setEnabled(haveSel)
        self.renameAction.setEnabled(haveSel)

        self.lockAction.setEnabled(haveSel)
        self.unlockAction.setEnabled(haveSel)
        self.recursivelyLockAction.setEnabled(haveSel)
        self.recursivelyUnlockAction.setEnabled(haveSel)

        self.deleteAction.setEnabled(haveSel)
*/
    contextMenu->exec(event->globalPos());
}

/* Not used even in the original CEED:

    def editSelectedWidgetName(self):
        selectedIndices = self.selectedIndexes()
        if len(selectedIndices) == 0:
            return
        self.setCurrentIndex(selectedIndices[0])
        self.edit(selectedIndices[0])

    def copySelectedWidgetPaths(self):
        selectedIndices = self.selectedIndexes()
        if len(selectedIndices) == 0:
            return

        paths = []
        for index in selectedIndices:
            item = self.model().itemFromIndex(index)
            if item.manipulator is not None:
                paths.append(item.manipulator.widget.getNamePath())

        if len(paths) > 0:
            # sort (otherwise the order is the item selection order)
            paths.sort()
            QtGui.QApplication.clipboard().setText(os.linesep.join(paths))

    def setSelectedWidgetsLocked(self, locked, recursive = False):
        selectedIndices = self.selectedIndexes()
        if len(selectedIndices) == 0:
            return

        # It is possible that we will make superfluous lock actions if user
        # selects widgets in a hierarchy (parent & child) and then does
        # a recursive lock. This doesn't do anything harmful so we don't
        # have any logic to prevent that.

        for index in selectedIndices:
            item = self.model().itemFromIndex(index)
            if item.manipulator is not None:
                item.setLocked(locked, recursive)
*/

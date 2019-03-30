#include "src/ui/layout/WidgetHierarchyTreeView.h"

WidgetHierarchyTreeView::WidgetHierarchyTreeView(QWidget* parent)
    : QTreeView(parent)
{

}

/*
    def __init__(self, parent = None):
        super(WidgetHierarchyTreeView, self).__init__(parent)

    def selectionChanged(self, selected, deselected):
        """Synchronizes tree selection with scene selection.
        """

        super(WidgetHierarchyTreeView, self).selectionChanged(selected, deselected)

        # we are running synchronization the other way, this prevents infinite loops and recursion
        if self.parentWidget.ignoreSelectionChanges:
            return

        self.parentWidget.visual.scene.ignoreSelectionChanges = True

        for index in selected.indexes():
            item = self.model().itemFromIndex(index)

            if isinstance(item, WidgetHierarchyItem):
                manipulatorPath = item.data(QtCore.Qt.UserRole)
                manipulator = None
                if manipulatorPath is not None:
                    manipulator = self.parentWidget.visual.scene.getManipulatorByPath(manipulatorPath)

                if manipulator is not None:
                    manipulator.setSelected(True)

        for index in deselected.indexes():
            item = self.model().itemFromIndex(index)

            if isinstance(item, WidgetHierarchyItem):
                manipulatorPath = item.data(QtCore.Qt.UserRole)
                manipulator = None
                if manipulatorPath is not None:
                    manipulator = self.parentWidget.visual.scene.getManipulatorByPath(manipulatorPath)

                if manipulator is not None:
                    manipulator.setSelected(False)

        self.parentWidget.visual.scene.ignoreSelectionChanges = False

    def setupContextMenu(self):
        self.setContextMenuPolicy(QtCore.Qt.DefaultContextMenu)

        self.contextMenu = QtGui.QMenu(self)

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

    def contextMenuEvent(self, event):
        selectedIndices = self.selectedIndexes()

        # TODO: since these actions enabled state depends on the selection,
        # move the enabling/disabling to a central "selection changed" handler.
        # The handler should be called on tab activations too because
        # activating a tab changes the selection, effectively.
        # We don't touch the cut/copy/paste actions because they're shared
        # among all editors and disabling them here would disable them
        # for the other editors too.
        haveSel = len(selectedIndices) > 0
        self.copyNamePathAction.setEnabled(haveSel)
        self.renameAction.setEnabled(haveSel)

        self.lockAction.setEnabled(haveSel)
        self.unlockAction.setEnabled(haveSel)
        self.recursivelyLockAction.setEnabled(haveSel)
        self.recursivelyUnlockAction.setEnabled(haveSel)

        self.deleteAction.setEnabled(haveSel)

        self.contextMenu.exec_(event.globalPos())

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

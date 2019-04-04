#include "src/ui/layout/WidgetHierarchyTreeModel.h"
#include "src/ui/layout/WidgetHierarchyItem.h"

WidgetHierarchyTreeModel::WidgetHierarchyTreeModel(WidgetHierarchyDockWidget* dockWidget)
    : _dockWidget(dockWidget)
{
    setSortRole(Qt::UserRole + 1);
    setItemPrototype(new WidgetHierarchyItem(nullptr));
}

/*

class WidgetHierarchyTreeModel(QtGui.QStandardItemModel):

    def data(self, index, role = QtCore.Qt.DisplayRole):
        return super(WidgetHierarchyTreeModel, self).data(index, role)

    def setData(self, index, value, role = QtCore.Qt.EditRole):
        if role == QtCore.Qt.EditRole:
            item = self.itemFromIndex(index)

            # if the new name is the same, cancel
            if value == item.manipulator.widget.getName():
                return False

            # validate the new name, cancel if invalid
            value = widgethelpers.Manipulator.getValidWidgetName(value)
            if not value:
                msgBox = QtGui.QMessageBox()
                msgBox.setText("The name was not changed because the new name is invalid.")
                msgBox.setIcon(QtGui.QMessageBox.Warning)
                msgBox.exec_()
                return False

            # check if the new name is unique in the parent, cancel if not
            parentWidget = item.manipulator.widget.getParent()
            if parentWidget is not None and parentWidget.isChild(value):
                msgBox = QtGui.QMessageBox()
                msgBox.setText("The name was not changed because the new name is in use by a sibling widget.")
                msgBox.setIcon(QtGui.QMessageBox.Warning)
                msgBox.exec_()
                return False

            # the name is good, apply it
            cmd = undo.RenameCommand(self.dockWidget.visual, item.manipulator.widget.getNamePath(), value)
            self.dockWidget.visual.tabbedEditor.undoStack.push(cmd)

            # return false because the undo command has changed the text of the item already
            return False

        return super(WidgetHierarchyTreeModel, self).setData(index, value, role)

    def flags(self, index):
        return super(WidgetHierarchyTreeModel, self).flags(index)

    def shouldManipulatorBeSkipped(self, manipulator):
        return \
           manipulator.widget.isAutoWindow() and \
           settings.getEntry("layout/visual/hide_deadend_autowidgets").value and \
           not manipulator.hasNonAutoWidgetDescendants()

    def constructSubtree(self, manipulator):
        ret = WidgetHierarchyItem(manipulator)

        manipulatorChildren = []

        for item in manipulator.childItems():
            if isinstance(item, widgethelpers.Manipulator):
                manipulatorChildren.append(item)

        for item in manipulatorChildren:
            if self.shouldManipulatorBeSkipped(item):
                # skip this branch as per settings
                continue

            childSubtree = self.constructSubtree(item)
            ret.appendRow(childSubtree)

        return ret

    def synchroniseSubtree(self, hierarchyItem, manipulator, recursive = True):
        """Attempts to synchronise subtree with given widget manipulator.
        If such a thing isn't possible it returns False.

        recursive - If True the synchronisation will recurse, trying to
                    unify child widget hierarchy items with child manipulators.
                    (This is generally what you want to do)
        """

        if hierarchyItem is None or manipulator is None:
            # no manipulator = no hierarchy item, we definitely can't synchronise
            return False

        if hierarchyItem.manipulator is not manipulator:
            # this widget hierarchy item itself will need to be recreated
            return False

        hierarchyItem.refreshPathData(False)

        if recursive:
            manipulatorsToRecreate = manipulator.getChildManipulators()

            i = 0
            # we knowingly do NOT use range in here, the rowCount might change
            # while we are processing!
            while i < hierarchyItem.rowCount():
                childHierarchyItem = hierarchyItem.child(i)

                if childHierarchyItem.manipulator in manipulatorsToRecreate and \
                   self.synchroniseSubtree(childHierarchyItem, childHierarchyItem.manipulator, True):
                    manipulatorsToRecreate.remove(childHierarchyItem.manipulator)
                    i += 1

                else:
                    hierarchyItem.removeRow(i)

            for childManipulator in manipulatorsToRecreate:
                if self.shouldManipulatorBeSkipped(childManipulator):
                    # skip this branch as per settings
                    continue

                hierarchyItem.appendRow(self.constructSubtree(childManipulator))

        hierarchyItem.refreshOrderingData(True, True)
        return True

    def getRootHierarchyItem(self):
        if self.rowCount() > 0:
            return self.item(0)

        else:
            return None

    def setRootManipulator(self, rootManipulator):
        if not self.synchroniseSubtree(self.getRootHierarchyItem(), rootManipulator):
            self.clear()

            if rootManipulator is not None:
                self.appendRow(self.constructSubtree(rootManipulator))

    def mimeData(self, indexes):
        # if the selection contains children of something that is also selected, we don't include that
        # (it doesn't make sense to move it anyways, it will be moved with its parent)

        def isChild(parent, potentialChild):
            i = 0
            # DFS, Qt doesn't have helper methods for this it seems to me :-/
            while i < parent.rowCount():
                child = parent.child(i)

                if child is potentialChild:
                    return True

                if isChild(child, potentialChild):
                    return True

                i += 1

            return False

        topItems = []

        for index in indexes:
            item = self.itemFromIndex(index)
            hasParent = False

            for parentIndex in indexes:
                if parentIndex is index:
                    continue

                potentialParent = self.itemFromIndex(parentIndex)

                if isChild(item, potentialParent):
                    hasParent = True
                    break

            if not hasParent:
                topItems.append(item)

        data = []
        for item in topItems:
            data.append(item.data(QtCore.Qt.UserRole))

        ret = QtCore.QMimeData()
        ret.setData("application/x-ceed-widget-paths", cPickle.dumps(data))

        return ret

    def mimeTypes(self):
        return ["application/x-ceed-widget-paths", "application/x-ceed-widget-type"]

    def dropMimeData(self, data, action, row, column, parent):
        if data.hasFormat("application/x-ceed-widget-paths"):
            # data.data(..).data() looks weird but is the correct thing!
            widgetPaths = cPickle.loads(data.data("application/x-ceed-widget-paths").data())
            targetWidgetPaths = []

            newParent = self.itemFromIndex(parent)
            if newParent is None:
                return False

            newParentManipulator = self.dockWidget.visual.scene.getManipulatorByPath(newParent.data(QtCore.Qt.UserRole))

            usedNames = set()
            for widgetPath in widgetPaths:
                oldWidgetName = widgetPath[widgetPath.rfind("/") + 1:]
                # Prevent name clashes at the new parent
                # When a name clash occurs, we suggest a new name to the user and
                # ask them to confirm it/enter their own.
                # The tricky part is that we have to consider the other widget renames
                # too (in case we're reparenting and renaming more than one widget)
                # and we must prevent invalid names (i.e. containing "/")
                suggestedName = oldWidgetName
                while True:
                    # get a name that's not used in the new parent, trying to keep
                    # the suggested name (which is the same as the old widget name at
                    # the beginning)
                    tempName = newParentManipulator.getUniqueChildWidgetName(suggestedName)
                    # if the name we got is the same as the one we wanted...
                    if tempName == suggestedName:
                        # ...we need to check our own usedNames list too, in case
                        # another widget we're reparenting has got this name.
                        counter = 2
                        while suggestedName in usedNames:
                            # When this happens, we simply add a numeric suffix to
                            # the suggested name. The result could theoretically
                            # collide in the new parent but it's OK because this
                            # is just a suggestion and will be checked again when
                            # the 'while' loops.
                            suggestedName = tempName + str(counter)
                            counter += 1
                            error = "Widget name is in use by another widget being " + ("copied" if action == QtCore.Qt.CopyAction else "moved")

                        # if we had no collision, we can keep this name!
                        if counter == 2:
                            break
                    else:
                        # The new parent had a child with that name already and so
                        # it gave us a new suggested name.
                        suggestedName = tempName
                        error = "Widget name already exists in the new parent"

                    # Ask the user to confirm our suggested name or enter a new one
                    # We do this in a loop because we validate the user input
                    while True:
                        suggestedName, ok = QtGui.QInputDialog.getText(
                                            self.dockWidget,
                                            error,
                                            "New name for '" + oldWidgetName + "':",
                                            QtGui.QLineEdit.Normal,
                                            suggestedName)
                        # Abort everything if the user cancels the dialog
                        if not ok:
                            return False
                        # Validate the entered name
                        suggestedName = widgethelpers.Manipulator.getValidWidgetName(suggestedName)
                        if suggestedName:
                            break
                        error = "Invalid name, please try again"

                usedNames.add(suggestedName)
                targetWidgetPaths.append(newParent.data(QtCore.Qt.UserRole) + "/" + suggestedName)

            if action == QtCore.Qt.MoveAction:
                cmd = undo.ReparentCommand(self.dockWidget.visual, widgetPaths, targetWidgetPaths)
                # FIXME: unreadable
                self.dockWidget.visual.tabbedEditor.undoStack.push(cmd)

                return True

            elif action == QtCore.Qt.CopyAction:
                # FIXME: TODO
                return False

        elif data.hasFormat("application/x-ceed-widget-type"):
            widgetType = data.data("application/x-ceed-widget-type").data()
            parentItem = self.itemFromIndex(parent)
            # if the drop was at empty space (parentItem is None) the parentItemPath
            # should be "" if no root item exists, otherwise the name of the root item
            parentItemPath = parentItem.data(QtCore.Qt.UserRole) if parentItem is not None else self.dockWidget.visual.scene.rootManipulator.widget.getName() if self.dockWidget.visual.scene.rootManipulator is not None else ""
            parentManipulator = self.dockWidget.visual.scene.getManipulatorByPath(parentItemPath) if parentItemPath else None
            uniqueName = parentManipulator.getUniqueChildWidgetName(widgetType.rsplit("/", 1)[-1]) if parentManipulator is not None else widgetType.rsplit("/", 1)[-1]

            cmd = undo.CreateCommand(self.dockWidget.visual, parentItemPath, widgetType, uniqueName)
            self.dockWidget.visual.tabbedEditor.undoStack.push(cmd)

            return True

        return False
*/

##############################################################################
#   created:    25th June 2014
#   author:     Lukas E Meindl
##############################################################################
##############################################################################
#   CEED - Unified CEGUI asset editor
#
#   Copyright (C) 2011-2014   Martin Preisler <martin@preisler.me>
#                             and contributing authors (see AUTHORS file)
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
##############################################################################

from PySide import QtCore
from PySide import QtGui

from ceed import action

from hierarchy_tree_item import LookNFeelHierarchyItem


class LookNFeelHierarchyTreeView(QtGui.QTreeView):
    """The WidgetLookFeel hierarchy tree definition
    This is based on the QTreeWidget widget.
    """

    def __init__(self, parent=None):
        super(LookNFeelHierarchyTreeView, self).__init__(parent)

        self.dockWidget = None

        self.originalBackgroundColour = QtGui.QColor(237, 215, 215)

    def drawRow(self, painter, option, index):
        """Draws alternating background colours for the items, changing the colour depending on the category.
        """

        from ceed.propertytree.ui import PropertyTreeView
        PropertyTreeView.paintAlternatingRowBackground(self, self.originalBackgroundColour, painter, option, index)

        # Calling the regular draw function
        super(LookNFeelHierarchyTreeView, self).drawRow(painter, option, index)

    def selectionChanged(self, selected, deselected):
        """Reacts on selection changes in the hierarchy tree by notifying the Falagrd element editor of the new selection.
        """

        super(LookNFeelHierarchyTreeView, self).selectionChanged(selected, deselected)

        # Notify the falagard element editor of the change
        selectedIndices = selected.indexes()
        if len(selectedIndices) > 0:
            firstItem = self.model().itemFromIndex(selectedIndices[0])
            self.dockWidget.tabbedEditor.visual.falagardElementEditorDockWidget.inspector.setSource(firstItem.falagardElement)
        else:
            self.dockWidget.tabbedEditor.visual.falagardElementEditorDockWidget.inspector.setSource(None)


        # we are running synchronization the other way, this prevents infinite loops and recursion
        if self.dockWidget.ignoreSelectionChanges:
            return

        self.dockWidget.visual.scene.ignoreSelectionChanges = True

        for index in selected.indexes():
            item = self.model().itemFromIndex(index)

            if isinstance(item, LookNFeelHierarchyItem):
                manipulatorPath = item.data(QtCore.Qt.UserRole)
                manipulator = None
                if manipulatorPath is not None:
                    manipulator = self.dockWidget.visual.scene.getManipulatorByPath(manipulatorPath)

                if manipulator is not None:
                    manipulator.setSelected(True)

        for index in deselected.indexes():
            item = self.model().itemFromIndex(index)

            if isinstance(item, LookNFeelHierarchyItem):
                manipulatorPath = item.data(QtCore.Qt.UserRole)
                manipulator = None
                if manipulatorPath is not None:
                    manipulator = self.dockWidget.visual.scene.getManipulatorByPath(manipulatorPath)

                if manipulator is not None:
                    manipulator.setSelected(False)

        self.dockWidget.visual.scene.ignoreSelectionChanges = False

    def setupContextMenu(self):
        self.setContextMenuPolicy(QtCore.Qt.DefaultContextMenu)

        self.contextMenu = QtGui.QMenu(self)

        self.contextMenu.addSeparator()

        self.cutAction = action.getAction("all_editors/cut")
        self.contextMenu.addAction(self.cutAction)
        self.copyAction = action.getAction("all_editors/copy")
        self.contextMenu.addAction(self.copyAction)
        self.pasteAction = action.getAction("all_editors/paste")
        self.contextMenu.addAction(self.pasteAction)
        self.deleteAction = action.getAction("all_editors/delete")
        self.contextMenu.addAction(self.deleteAction)

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

        self.deleteAction.setEnabled(haveSel)

        self.contextMenu.exec_(event.globalPos())
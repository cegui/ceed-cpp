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
import cPickle

import PyCEGUI

from ceed import resizable

from ceed.editors import multi

from ceed.cegui import widgethelpers as cegui_widgethelpers

from ceed.editors.looknfeel import undoable_commands

from ceed.editors.looknfeel.hierarchy_dock_widget import LookNFeelHierarchyDockWidget
from ceed.editors.looknfeel.falagard_element_editor import LookNFeelFalagardElementEditorDockWidget
from ceed.editors.looknfeel.falagard_element_inspector import FalagardElementAttributesManager


class LookNFeelVisualEditing(QtGui.QWidget, multi.EditMode):
    """This is the default visual editing mode

    see ceed.editors.multi.EditMode
    """

    def __init__(self, tabbedEditor):
        """
        :param tabbedEditor: LookNFeelTabbedEditor
        :return:
        """
        super(LookNFeelVisualEditing, self).__init__()

        self.tabbedEditor = tabbedEditor
        self.rootWindow = None

        self.lookNFeelHierarchyDockWidget = LookNFeelHierarchyDockWidget(self, tabbedEditor)
        self.lookNFeelWidgetLookSelectorWidget = LookNFeelWidgetLookSelectorWidget(self, tabbedEditor)
        self.falagardElementEditorDockWidget = LookNFeelFalagardElementEditorDockWidget(self, tabbedEditor)

        looknfeel = QtGui.QVBoxLayout(self)
        looknfeel.setContentsMargins(0, 0, 0, 0)
        self.setLayout(looknfeel)

        self.scene = EditingScene(self)

        self.setupActions()
        self.setupToolBar()
        self.lookNFeelHierarchyDockWidget.treeView.setupContextMenu()

    def initialise(self):
        propertyMap = mainwindow.MainWindow.instance.project.propertyMap
        widgetLookPropertyManager = FalagardElementAttributesManager(propertyMap, self)
        self.falagardElementEditorDockWidget.inspector.setPropertyManager(widgetLookPropertyManager)

        self.rootWindow = PyCEGUI.WindowManager.getSingleton().createWindow("DefaultWindow", "LookNFeelEditorRoot")
        PyCEGUI.System.getSingleton().getDefaultGUIContext().setRootWindow(self.rootWindow)

    def destroy(self):
        # Remove the widget with the previous WidgetLook from the scene
        self.destroyCurrentPreviewWidget()

    def setupActions(self):
        self.connectionGroup = action.ConnectionGroup(action.ActionManager.instance)

    def setupToolBar(self):
        self.toolBar = QtGui.QToolBar("looknfeel")
        self.toolBar.setObjectName("looknfeelToolbar")
        self.toolBar.setIconSize(QtCore.QSize(32, 32))

    def rebuildEditorMenu(self, editorMenu):
        """Adds actions to the editor menu"""

    def destroyCurrentPreviewWidget(self):
        """
        Destroys all child windows of the root, which means that all preview windows of the selected WidgetLookFeel should be destroyed
        :return:
        """

        if self.rootWindow is None:
            return

        # Remove the widget with the previous WidgetLook from the scene
        while self.rootWindow.getChildCount() != 0:
            PyCEGUI.WindowManager.getSingleton().destroyWindow(self.rootWindow.getChildAtIdx(0))

        # TODO (Ident) :
        # Fix the window pool cleanup issues in CEGUI default and remove this later in the CEED default branch
        # for more info see: http://cegui.org.uk/wiki/The_Lederhosen_project_-_The_Second_Coming
        PyCEGUI.WindowManager.getSingleton().cleanDeadPool()

    def updateWidgetLookPreview(self):
        self.destroyCurrentPreviewWidget()

        if self.tabbedEditor.targetWidgetLook:

            # Add new widget representing the new WidgetLook to the scene, if the factory is registered
            factoryPresent = PyCEGUI.WindowFactoryManager.getSingleton().isFactoryPresent(self.tabbedEditor.targetWidgetLook)
            if factoryPresent:
                widgetLookWindow = PyCEGUI.WindowManager.getSingleton().createWindow(self.tabbedEditor.targetWidgetLook, "WidgetLookWindow")
                self.rootWindow.addChild(widgetLookWindow)
            else:
                self.tabbedEditor.targetWidgetLook = ""

        #Refresh the drawing of the preview
        self.scene.update()

    def updateToNewTargetWidgetLook(self):
        self.updateWidgetLookPreview()

        self.falagardElementEditorDockWidget.inspector.setSource(None)

        self.lookNFeelHierarchyDockWidget.updateToNewWidgetLook(self.tabbedEditor.targetWidgetLook)

    def showEvent(self, event):
        mainwindow.MainWindow.instance.ceguiContainerWidget.activate(self, self.scene)
        mainwindow.MainWindow.instance.ceguiContainerWidget.setViewFeatures(wheelZoom = True,
                                                                            middleButtonScroll = True,
                                                                            continuousRendering = settings.getEntry("looknfeel/visual/continuous_rendering").value)

        self.lookNFeelHierarchyDockWidget.setEnabled(True)
        self.lookNFeelWidgetLookSelectorWidget.setEnabled(True)
        self.falagardElementEditorDockWidget.setEnabled(True)

        PyCEGUI.System.getSingleton().getDefaultGUIContext().setRootWindow(self.rootWindow)

        self.toolBar.setEnabled(True)
        if self.tabbedEditor.editorMenu() is not None:
            self.tabbedEditor.editorMenu().menuAction().setEnabled(True)

        # connect all our actions
        self.connectionGroup.connectAll()

        super(LookNFeelVisualEditing, self).showEvent(event)

    def hideEvent(self, event):
        # disconnected all our actions
        self.connectionGroup.disconnectAll()

        self.lookNFeelHierarchyDockWidget.setEnabled(False)
        self.lookNFeelWidgetLookSelectorWidget.setEnabled(False)
        self.falagardElementEditorDockWidget.setEnabled(False)

        self.toolBar.setEnabled(False)
        if self.tabbedEditor.editorMenu() is not None:
            self.tabbedEditor.editorMenu().menuAction().setEnabled(False)

        mainwindow.MainWindow.instance.ceguiContainerWidget.deactivate(self)

        super(LookNFeelVisualEditing, self).hideEvent(event)

    def focusPropertyInspectorFilterBox(self):
        """Focuses into property set inspector filter

        This potentially allows the user to just press a shortcut to find properties to edit,
        instead of having to reach for a mouse.
        """

        filterBox = self.propertiesDockWidget.inspector.filterBox
        # selects all contents of the filter so that user can replace that with their search phrase
        filterBox.selectAll()
        # sets focus so that typing puts text into the filter box without clicking
        filterBox.setFocus()

    def performCut(self):
        ret = self.performCopy()
        self.scene.deleteSelectedWidgets()

        return ret


class LookNFeelWidgetLookSelectorWidget(QtGui.QDockWidget):
    """This dock widget allows to select a WidgetLook from a combobox and start editing it
    """

    def __init__(self, visual, tabbedEditor):
        """
        :param visual: LookNFeelVisualEditing
        :param tabbedEditor: LookNFeelTabbedEditor
        :return:
        """
        super(LookNFeelWidgetLookSelectorWidget, self).__init__()

        self.tabbedEditor = tabbedEditor

        self.visual = visual

        self.ui = ceed.ui.editors.looknfeel.looknfeelwidgetlookselectorwidget.Ui_LookNFeelWidgetLookSelector()
        self.ui.setupUi(self)

        self.fileNameLabel = self.findChild(QtGui.QLabel, "fileNameLabel")
        """:type : QtGui.QLabel"""

        self.widgetLookNameBox = self.findChild(QtGui.QComboBox, "widgetLookNameBox")
        """:type : QtGui.QComboBox"""

        self.editWidgetLookButton = self.findChild(QtGui.QPushButton, "editWidgetLookButton")
        self.editWidgetLookButton.pressed.connect(self.slot_editWidgetLookButtonPressed)

    def resizeEvent(self, QResizeEvent):
        self.setFileNameLabel()

        super(LookNFeelWidgetLookSelectorWidget, self).resizeEvent(QResizeEvent)

    def slot_editWidgetLookButtonPressed(self):
        # Handles the actions necessary after a user selects a new WidgetLook to edit
        selectedItemIndex = self.widgetLookNameBox.currentIndex()
        selectedWidgetLookName = self.widgetLookNameBox.itemData(selectedItemIndex)

        command = undoable_commands.TargetWidgetChangeCommand(self.visual, self.tabbedEditor, selectedWidgetLookName)
        self.tabbedEditor.undoStack.push(command)

    def populateWidgetLookComboBox(self, widgetLookNameTuples):
        self.widgetLookNameBox.clear()
        # We populate the combobox with items that use the original name as display text but have the name of the live-editable WidgetLook stored as a QVariant

        for nameTuple in widgetLookNameTuples:
            self.widgetLookNameBox.addItem(nameTuple[0], nameTuple[1])

    def setFileNameLabel(self):
        # Shortens the file name so that it fits into the label and ends with "...", the full path is set as a tooltip
        fileNameStr = self.tabbedEditor.filePath
        fontMetrics = self.fileNameLabel.fontMetrics()
        labelWidth = self.fileNameLabel.size().width()
        fontMetricsWidth = fontMetrics.width(fileNameStr)
        rightMargin = 6
        if labelWidth < fontMetricsWidth:
            self.fileNameLabel.setText(fontMetrics.elidedText(fileNameStr, QtCore.Qt.ElideRight, labelWidth - rightMargin))
        else:
            self.fileNameLabel.setText(fileNameStr)

        self.fileNameLabel.setToolTip(fileNameStr)


class EditingScene(cegui_widgethelpers.GraphicsScene):
    """This scene contains all the manipulators users want to interact it. You can visualise it as the
    visual editing centre screen where CEGUI is rendered.

    It renders CEGUI on it's background and outlines (via Manipulators) in front of it.
    """

    def __init__(self, visual):
        super(EditingScene, self).__init__(mainwindow.MainWindow.instance.ceguiInstance)

        self.visual = visual
        self.ignoreSelectionChanges = False
        self.selectionChanged.connect(self.slot_selectionChanged)

    def setCEGUIDisplaySize(self, width, height, lazyUpdate = True):
        # overridden to keep the manipulators in sync

        super(EditingScene, self).setCEGUIDisplaySize(width, height, lazyUpdate)

    def slot_selectionChanged(self):
        selection = self.selectedItems()

        sets = []
        for item in selection:
            wdt = None

            if isinstance(item, widgethelpers.Manipulator):
                wdt = item.widget

            elif isinstance(item, resizable.ResizingHandle):
                if isinstance(item.parentResizable, widgethelpers.Manipulator):
                    wdt = item.parentResizable.widget

            if wdt is not None and wdt not in sets:
                sets.append(wdt)

    def mouseReleaseEvent(self, event):
        super(EditingScene, self).mouseReleaseEvent(event)

        movedWidgetPaths = []
        movedOldPositions = {}
        movedNewPositions = {}

        resizedWidgetPaths = []
        resizedOldPositions = {}
        resizedOldSizes = {}
        resizedNewPositions = {}
        resizedNewSizes = {}

        # we have to "expand" the items, adding parents of resizing handles
        # instead of the handles themselves
        expandedSelectedItems = []
        for selectedItem in self.selectedItems():
            if isinstance(selectedItem, widgethelpers.Manipulator):
                expandedSelectedItems.append(selectedItem)
            elif isinstance(selectedItem, resizable.ResizingHandle):
                if isinstance(selectedItem.parentItem(), widgethelpers.Manipulator):
                    expandedSelectedItems.append(selectedItem.parentItem())

        for item in expandedSelectedItems:
            if isinstance(item, widgethelpers.Manipulator):
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

        if len(movedWidgetPaths) > 0:
            cmd = undoable_commands.MoveCommand(self.visual, movedWidgetPaths, movedOldPositions, movedNewPositions)
            self.visual.tabbedEditor.undoStack.push(cmd)

        if len(resizedWidgetPaths) > 0:
            cmd = undoable_commands.ResizeCommand(self.visual, resizedWidgetPaths, resizedOldPositions, resizedOldSizes, resizedNewPositions, resizedNewSizes)
            self.visual.tabbedEditor.undoStack.push(cmd)

    def keyReleaseEvent(self, event):
        handled = False

        if event.key() == QtCore.Qt.Key_Delete:
            handled = self.deleteSelectedWidgets()

        if not handled:
            super(EditingScene, self).keyReleaseEvent(event)

        else:
            event.accept()

# needs to be at the end to sort circular deps
import ceed.ui.editors.looknfeel.looknfeelhierarchydockwidget
import ceed.ui.editors.looknfeel.looknfeelwidgetlookselectorwidget
import ceed.ui.editors.looknfeel.looknfeelpropertyeditordockwidget

# needs to be at the end, import to get the singleton
from ceed import mainwindow
from ceed import settings
from ceed import action

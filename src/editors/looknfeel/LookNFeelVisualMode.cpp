#include "src/editors/looknfeel/LookNFeelVisualMode.h"

LookNFeelVisualMode::LookNFeelVisualMode()
{

}

void LookNFeelVisualMode::setupActions()
{
/*
    cat = actionManager.createCategory(name = "looknfeel", label = "Look n' Feel Editor")

    cat.createAction(name = "align_hleft", label = "Align &Left (horizontally)",
                     help_ = "Sets horizontal alignment of all selected widgets to left.",
                     icon = QtGui.QIcon("icons/looknfeel_editing/align_hleft.png"))
    cat.createAction(name = "align_hcentre", label = "Align Centre (&horizontally)",
                     help_ = "Sets horizontal alignment of all selected widgets to centre.",
                     icon = QtGui.QIcon("icons/looknfeel_editing/align_hcentre.png"))
    cat.createAction(name = "align_hright", label = "Align &Right (horizontally)",
                     help_ = "Sets horizontal alignment of all selected widgets to right.",
                     icon = QtGui.QIcon("icons/looknfeel_editing/align_hright.png"))

    cat.createAction(name = "align_vtop", label = "Align &Top (vertically)",
                     help_ = "Sets vertical alignment of all selected widgets to top.",
                     icon = QtGui.QIcon("icons/looknfeel_editing/align_vtop.png"))
    cat.createAction(name = "align_vcentre", label = "Align Centre (&vertically)",
                     help_ = "Sets vertical alignment of all selected widgets to centre.",
                     icon = QtGui.QIcon("icons/looknfeel_editing/align_vcentre.png"))
    cat.createAction(name = "align_vbottom", label = "Align &Bottom (vertically)",
                     help_ = "Sets vertical alignment of all selected widgets to bottom.",
                     icon = QtGui.QIcon("icons/looknfeel_editing/align_vbottom.png"))

    cat.createAction(name = "snap_grid", label = "Snap to &Grid",
                     help_ = "When resizing and moving widgets, if checked this makes sure they snap to a snap grid (see settings for snap grid related entries), also shows the snap grid if checked.",
                     icon = QtGui.QIcon("icons/looknfeel_editing/snap_grid.png"),
                     defaultShortcut = QtGui.QKeySequence(QtCore.Qt.Key_Space)).setCheckable(True)

    absolute_mode = cat.createAction(
                     name = "absolute_mode", label = "&Absolute Resizing && Moving Deltas",
                     help_ = "When resizing and moving widgets, if checked this makes the delta absolute, it is relative if unchecked.",
                     icon = QtGui.QIcon("icons/looknfeel_editing/absolute_mode.png"),
                     defaultShortcut = QtGui.QKeySequence(QtCore.Qt.Key_A))
    absolute_mode.setCheckable(True)
    absolute_mode.setChecked(True)

    cat.createAction(name = "normalise_position", label = "Normalise &Position (cycle)",
                     help_ = "If the position is mixed (absolute and relative) it becomes relative only, if it's relative it becomes absolute, if it's absolute it becomes relative.",
                     icon = QtGui.QIcon("icons/looknfeel_editing/normalise_position.png"),
                     defaultShortcut = QtGui.QKeySequence(QtCore.Qt.Key_D))

    cat.createAction(name = "normalise_size", label = "Normalise &Size (cycle)",
                     help_ = "If the size is mixed (absolute and relative) it becomes relative only, if it's relative it becomes absolute, if it's absolute it becomes relative.",
                     icon = QtGui.QIcon("icons/looknfeel_editing/normalise_size.png"),
                     defaultShortcut = QtGui.QKeySequence(QtCore.Qt.Key_S))

    cat.createAction(name = "focus_property_inspector_filter_box", label = "&Focus Property Inspector Filter Box",
                     help_ = "This allows you to easily press a shortcut and immediately search through properties without having to reach for a mouse.",
                     icon = QtGui.QIcon("icons/looknfeel_editing/focus_property_inspector_filter_box.png"),
                     defaultShortcut = QtGui.QKeySequence(QtGui.QKeySequence.Find))
*/
}

/*

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
*/

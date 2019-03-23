#include "src/editors/imageset/ImagesetEditor.h"
#include "src/editors/imageset/ImagesetCodeMode.h"

ImagesetEditor::ImagesetEditor(const QString& filePath)
    : MultiModeEditor(/*imageset_compatibility.manager, */ filePath)
{
/*
        self.visual = visual.VisualEditing(self)
        self.addTab(self.visual, "Visual")
*/
    auto codeMode = new ImagesetCodeMode(*this);
    tabs.addTab(codeMode, "Code");
/*
        # set the toolbar icon size according to the setting and subscribe to it
        self.tbIconSizeEntry = settings.getEntry("global/ui/toolbar_icon_size")
        self.updateToolbarSize(self.tbIconSizeEntry.value)
        self.tbIconSizeCallback = lambda value: self.updateToolbarSize(value)
        self.tbIconSizeEntry.subscribe(self.tbIconSizeCallback)
*/
}

void ImagesetEditor::initialize()
{
    MultiModeEditor::initialize();

/*
        root = None
        try:
            root = ElementTree.fromstring(self.nativeData)

        except:
            # things didn't go smooth
            # 2 reasons for that
            #  * the file is empty
            #  * the contents of the file are invalid
            #
            # In the first case we will silently move along (it is probably just a new file),
            # in the latter we will output a message box informing about the situation

            # the file should exist at this point, so we are not checking and letting exceptions
            # fly out of this method
            if os.path.getsize(self.filePath) > 2:
                # the file contains more than just CR LF
                QtGui.QMessageBox.question(self,
                                           "Can't parse given imageset!",
                                           "Parsing '%s' failed, it's most likely not a valid XML file. "
                                           "Constructing empty imageset instead (if you save you will override the invalid data!). "
                                           "Exception details follow:\n%s" % (self.filePath, sys.exc_info()[1]),
                                           QtGui.QMessageBox.Ok)

            # we construct the minimal empty imageset
            root = ElementTree.Element("Imageset")
            root.set("Name", "")
            root.set("Imagefile", "")

        self.visual.initialise(root)
*/
}

void ImagesetEditor::finalize()
{
    // Unsubscribe from the toolbar icon size setting
/*
        self.tbIconSizeEntry.unsubscribe(self.tbIconSizeCallback)
*/
    MultiModeEditor::finalize();
}

void ImagesetEditor::activate(QMenu* editorMenu)
{
    MultiModeEditor::activate(editorMenu);
/*
        self.mainWindow.addToolBar(QtCore.Qt.ToolBarArea.TopToolBarArea, self.visual.toolBar)
        self.visual.toolBar.show()

        self.mainWindow.addDockWidget(QtCore.Qt.LeftDockWidgetArea, self.visual.dockWidget)
        self.visual.dockWidget.setVisible(True)
*/
}

void ImagesetEditor::setupEditorMenu(QMenu* editorMenu)
{
/*
        editorMenu.setTitle("&Imageset")
        self.visual.rebuildEditorMenu(editorMenu)

        return True, self.currentWidget() == self.visual
        // visible, enabled
*/
}

void ImagesetEditor::deactivate()
{
    /*
        self.mainWindow.removeDockWidget(self.visual.dockWidget)
        self.mainWindow.removeToolBar(self.visual.toolBar)
    */
    MultiModeEditor::deactivate();
}

/*
    def updateToolbarSize(self, size):
        if size < 16:
            size = 16
        self.visual.toolBar.setIconSize(QtCore.QSize(size, size))

    def saveAs(self, targetPath, updateCurrentPath = True):
        codeMode = self.currentWidget() is self.code

        # if user saved in code mode, we process the code by propagating it to visual
        # (allowing the change propagation to do the code validating and other work for us)

        if codeMode:
            self.code.propagateToVisual()

        rootElement = self.visual.imagesetEntry.saveToElement()
        # we indent to make the resulting files as readable as possible
        xmledit.indent(rootElement)

        self.nativeData = ElementTree.tostring(rootElement, "utf-8")

        return super(ImagesetTabbedEditor, self).saveAs(targetPath, updateCurrentPath)

    def performCut(self):
        if self.currentWidget() is self.visual:
            return self.visual.performCut()

        return False

    def performCopy(self):
        if self.currentWidget() is self.visual:
            return self.visual.performCopy()

        return False

    def performPaste(self):
        if self.currentWidget() is self.visual:
            return self.visual.performPaste()

        return False

    def performDelete(self):
        if self.currentWidget() is self.visual:
            return self.visual.performDelete()

        return False

    def zoomIn(self):
        if self.currentWidget() is self.visual:
            self.visual.zoomIn()

    def zoomOut(self):
        if self.currentWidget() is self.visual:
            self.visual.zoomOut()

    def zoomReset(self):
        if self.currentWidget() is self.visual:
            self.visual.zoomOriginal()
*/

/*

def declare(actionManager):
    cat = actionManager.createCategory(name = "imageset", label = "Imageset Editor")

    cat.createAction(name = "edit_offsets", label = "Edit &Offsets",
                     help_ = "When you select an image definition, a crosshair will appear in it representing it's offset centrepoint.",
                     icon = QtGui.QIcon("icons/imageset_editing/edit_offsets.png"),
                     defaultShortcut = QtGui.QKeySequence(QtCore.Qt.Key_Space)).setCheckable(True)

    cat.createAction(name = "cycle_overlapping", label = "Cycle O&verlapping Image Definitions",
                     help_ = "When images definition overlap in such a way that makes it hard/impossible to select the definition you want, this allows you to select on of them and then just cycle until the right one is selected.",
                     icon = QtGui.QIcon("icons/imageset_editing/cycle_overlapping.png"),
                     defaultShortcut = QtGui.QKeySequence(QtCore.Qt.Key_Q))

    cat.createAction(name = "create_image", label = "&Create Image Definition",
                     help_ = "Creates a new image definition at the current cursor position, sized 50x50 pixels.",
                     icon = QtGui.QIcon("icons/imageset_editing/create_image.png"))

    cat.createAction(name = "duplicate_image", label = "&Duplicate Image Definition",
                     help_ = "Duplicates selected image definitions.",
                     icon = QtGui.QIcon("icons/imageset_editing/duplicate_image.png"))

    cat.createAction(name = "focus_image_list_filter_box", label = "&Focus Image Definition List Filter Box",
                     help_ = "This allows you to easily press a shortcut and immediately search through image definitions without having to reach for a mouse.",
                     icon = QtGui.QIcon("icons/imageset_editing/focus_image_list_filter_box.png"),
                     defaultShortcut = QtGui.QKeySequence(QtGui.QKeySequence.Find))

def declare(settings):
    category = settings.createCategory(name = "imageset", label = "Imageset editing")

    visual = category.createSection(name = "visual", label = "Visual editing")

    visual.createEntry(name = "overlay_image_labels", type_ = bool, label = "Show overlay labels of images",
                    help_ = "Show overlay labels of images.",
                    defaultValue = True, widgetHint = "checkbox",
                    sortingWeight = 1)

    visual.createEntry(name = "partial_updates", type_ = bool, label = "Use partial drawing updates",
                    help_ = "Will use partial 2D updates using accelerated 2D machinery. The performance of this is very dependent on your platform and hardware. MacOSX handles partial updates much better than Linux it seems. If you have a very good GPU, don't tick this.",
                    defaultValue = False, widgetHint = "checkbox", changeRequiresRestart = True,
                    sortingWeight = 2)
*/

//---------------------------------------------------------------------

QString ImagesetEditorFactory::getFileTypesDescription() const
{
    return "Imageset files";
}

QStringList ImagesetEditorFactory::getFileExtensions() const
{
    /*
        extensions = imageset_compatibility.manager.getAllPossibleExtensions()
    */
    return { "imageset" };
}

EditorBasePtr ImagesetEditorFactory::create(const QString& filePath) const
{
    return std::make_unique<ImagesetEditor>(filePath);
}

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

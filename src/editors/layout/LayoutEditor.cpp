#include "src/editors/layout/LayoutEditor.h"
#include "src/editors/layout/LayoutCodeMode.h"

LayoutEditor::LayoutEditor(const QString& filePath)
    : MultiModeEditor(/*layout_compatibility.manager, */ filePath)
{
/*
        self.visual = visual.VisualEditing(self)
        self.addTab(self.visual, "Visual")

*/
    auto codeMode = new LayoutCodeMode(*this);
    tabs.addTab(codeMode, "Code");
/*

        # Layout Previewer is not actually an edit mode, you can't edit the layout from it,
        # however for everything to work smoothly we do push edit mode changes to it to the
        # undo stack.
        #
        # TODO: This could be improved at least a little bit if 2 consecutive edit mode changes
        #       looked like this: A->Preview, Preview->C.  We could simply turn this into A->C,
        #       and if A = C it would eat the undo command entirely.
        self.previewer = preview.LayoutPreviewer(self)
        self.addTab(self.previewer, "Live Preview")

        # set the toolbar icon size according to the setting and subscribe to it
        self.tbIconSizeEntry = settings.getEntry("global/ui/toolbar_icon_size")
        self.updateToolbarSize(self.tbIconSizeEntry.value)
        self.tbIconSizeCallback = lambda value: self.updateToolbarSize(value)
        self.tbIconSizeEntry.subscribe(self.tbIconSizeCallback)
*/
}

void LayoutEditor::initialize()
{
    MultiModeEditor::initialize();
/*
        # we have to make the context the current context to ensure textures are fine
        self.mainWindow.ceguiContainerWidget.makeGLContextCurrent()

        root = None
        if self.nativeData != "":
            root = PyCEGUI.WindowManager.getSingleton().loadLayoutFromString(self.nativeData)

        self.visual.initialise(root)
*/
}

void LayoutEditor::finalize()
{
/* was in destroy!
        # unsubscribe from the toolbar icon size setting
        self.tbIconSizeEntry.unsubscribe(self.tbIconSizeCallback)
*/
    MultiModeEditor::finalize();
}

void LayoutEditor::activate(QMenu* editorMenu)
{
    MultiModeEditor::activate(editorMenu);

/*
        self.mainWindow.addDockWidget(QtCore.Qt.LeftDockWidgetArea, self.visual.hierarchyDockWidget)
        self.visual.hierarchyDockWidget.setVisible(True)
        self.mainWindow.addDockWidget(QtCore.Qt.RightDockWidgetArea, self.visual.propertiesDockWidget)
        self.visual.propertiesDockWidget.setVisible(True)
        self.mainWindow.addDockWidget(QtCore.Qt.LeftDockWidgetArea, self.visual.createWidgetDockWidget)
        self.visual.createWidgetDockWidget.setVisible(True)
        self.mainWindow.addToolBar(QtCore.Qt.ToolBarArea.TopToolBarArea, self.visual.toolBar)
        self.visual.toolBar.show()
*/
}

void LayoutEditor::setupEditorMenu(QMenu* editorMenu)
{
/*
        editorMenu.setTitle("&Layout")
        self.visual.rebuildEditorMenu(editorMenu)

        return True, self.currentWidget() == self.visual
        // visible, enabled
*/
}

void LayoutEditor::deactivate()
{
/*
        self.mainWindow.removeDockWidget(self.visual.hierarchyDockWidget)
        self.mainWindow.removeDockWidget(self.visual.propertiesDockWidget)
        self.mainWindow.removeDockWidget(self.visual.createWidgetDockWidget)
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

        currentRootWidget = self.visual.getCurrentRootWidget()

        if currentRootWidget is None:
            QtGui.QMessageBox.warning(self.mainWindow, "No root widget in the layout!", "I am refusing to save your layout, CEGUI layouts are invalid unless they have a root widget!\n\nPlease create a root widget before saving.")
            return False

        self.nativeData = PyCEGUI.WindowManager.getSingleton().getLayoutAsString(currentRootWidget)
        return super(LayoutTabbedEditor, self).saveAs(targetPath, updateCurrentPath)

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
            self.visual.scene.views()[0].zoomIn()

    def zoomOut(self):
        if self.currentWidget() is self.visual:
            self.visual.scene.views()[0].zoomOut()

    def zoomReset(self):
        if self.currentWidget() is self.visual:
            self.visual.scene.views()[0].zoomOriginal()
*/

//---------------------------------------------------------------------

QString LayoutEditorFactory::getFileTypesDescription() const
{
    return "Layout files";
}

QStringList LayoutEditorFactory::getFileExtensions() const
{
    /*
        extensions = layout_compatibility.manager.getAllPossibleExtensions()
    */
    return { "layout" };
}

EditorBasePtr LayoutEditorFactory::create(const QString& filePath) const
{
    return std::make_unique<LayoutEditor>(filePath);
}

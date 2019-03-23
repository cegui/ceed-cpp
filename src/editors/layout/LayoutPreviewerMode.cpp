#include "src/editors/layout/LayoutPreviewerMode.h"

LayoutPreviewerMode::LayoutPreviewerMode(MultiModeEditor& editor, QWidget* parent)
    : QWidget(parent)
    //!!!, IEditMode(editor) //???EditModeBase?
{
/*
        self.tabbedEditor = tabbedEditor
        self.rootWidget = None

        layout = QtGui.QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        self.setLayout(layout)
*/
}

/*
    def activate(self):
        super(LayoutPreviewer, self).activate()

        assert(self.rootWidget is None)

        # we have to make the context the current context to ensure textures are fine
        mainwindow.MainWindow.instance.ceguiContainerWidget.makeGLContextCurrent()

        currentRootWidget = self.tabbedEditor.visual.getCurrentRootWidget()
        if currentRootWidget is None:
            self.rootWidget = None

        else:
            # lets clone so we don't affect the layout at all
            self.rootWidget = currentRootWidget.clone()

        PyCEGUI.System.getSingleton().getDefaultGUIContext().setRootWindow(self.rootWidget)

    def deactivate(self):
        if self.rootWidget is not None:
            PyCEGUI.WindowManager.getSingleton().destroyWindow(self.rootWidget)
            self.rootWidget = None

        return super(LayoutPreviewer, self).deactivate()

    def showEvent(self, event):
        super(LayoutPreviewer, self).showEvent(event)

        mainwindow.MainWindow.instance.ceguiContainerWidget.activate(self)
        # we always want continuous rendering in live preview
        mainwindow.MainWindow.instance.ceguiContainerWidget.setViewFeatures(continuousRendering = True)
        mainwindow.MainWindow.instance.ceguiContainerWidget.enableInput()

        if self.rootWidget:
            PyCEGUI.System.getSingleton().getDefaultGUIContext().setRootWindow(self.rootWidget)

    def hideEvent(self, event):
        mainwindow.MainWindow.instance.ceguiContainerWidget.disableInput()
        mainwindow.MainWindow.instance.ceguiContainerWidget.deactivate(self)

        if self.rootWidget:
            PyCEGUI.System.getSingleton().getDefaultGUIContext().setRootWindow(None)

        super(LayoutPreviewer, self).hideEvent(event)
*/

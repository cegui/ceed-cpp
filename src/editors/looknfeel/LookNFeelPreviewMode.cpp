#include "src/editors/looknfeel/LookNFeelPreviewMode.h"
#include "src/editors/looknfeel/LookNFeelEditor.h"
#include <qboxlayout.h>

LookNFeelPreviewMode::LookNFeelPreviewMode(LookNFeelEditor& editor)
    : IEditMode(editor)
{
/*
        self.rootWidget = None
*/
    auto looknfeel = new QVBoxLayout(this);
    looknfeel->setContentsMargins(0, 0, 0, 0);
    setLayout(looknfeel);
}

/*
    def activate(self):
        super(LookNFeelPreviewer, self).activate()

        assert(self.rootWidget is None)

        # we have to make the context the current context to ensure textures are fine
        mainwindow.MainWindow.instance.ceguiContainerWidget.makeGLContextCurrent()

        currentRootWindow = self.tabbedEditor.visual.rootWindow
        if currentRootWindow is None:
            self.rootWidget = None

        else:
            # lets clone so we don't affect the Look n' Feel at all
            self.rootWidget = currentRootWindow.clone()

        PyCEGUI.System.getSingleton().getDefaultGUIContext().setRootWindow(self.rootWidget)

    def deactivate(self):
        if self.rootWidget is not None:
            PyCEGUI.WindowManager.getSingleton().destroyWindow(self.rootWidget)
            self.rootWidget = None

        return super(LookNFeelPreviewer, self).deactivate()

    def showEvent(self, event):
        super(LookNFeelPreviewer, self).showEvent(event)

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

        super(LookNFeelPreviewer, self).hideEvent(event)
*/

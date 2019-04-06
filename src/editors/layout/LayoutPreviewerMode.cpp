#include "src/editors/layout/LayoutPreviewerMode.h"
#include "qboxlayout.h"

LayoutPreviewerMode::LayoutPreviewerMode(MultiModeEditor& editor, QWidget* parent)
    : QWidget(parent)
    , IEditMode(editor)
{
/*
        self.rootWidget = None
*/
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
}

void LayoutPreviewerMode::activate()
{
    IEditMode::activate();

/*
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
*/
}

bool LayoutPreviewerMode::deactivate()
{
/*
        if self.rootWidget is not None:
            PyCEGUI.WindowManager.getSingleton().destroyWindow(self.rootWidget)
            self.rootWidget = None
*/
    return IEditMode::deactivate();
}

void LayoutPreviewerMode::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

/*
        mainwindow.MainWindow.instance.ceguiContainerWidget.activate(self)
        // We always want continuous rendering in live preview
        mainwindow.MainWindow.instance.ceguiContainerWidget.setViewFeatures(continuousRendering = True)
        mainwindow.MainWindow.instance.ceguiContainerWidget.enableInput()

        if self.rootWidget:
            PyCEGUI.System.getSingleton().getDefaultGUIContext().setRootWindow(self.rootWidget)
*/
}

void LayoutPreviewerMode::hideEvent(QHideEvent* event)
{
/*
        mainwindow.MainWindow.instance.ceguiContainerWidget.disableInput()
        mainwindow.MainWindow.instance.ceguiContainerWidget.deactivate(self)

        if self.rootWidget:
            PyCEGUI.System.getSingleton().getDefaultGUIContext().setRootWindow(None)
*/
    QWidget::hideEvent(event);
}

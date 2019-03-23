#include "src/editors/layout/LayoutCodeMode.h"

LayoutCodeMode::LayoutCodeMode(MultiModeEditor& editor)
    : ViewRestoringCodeEditMode(editor)
{

}

QString LayoutCodeMode::getNativeCode()
{
/*
        currentRootWidget = self.tabbedEditor.visual.getCurrentRootWidget()

        if currentRootWidget is None:
            return ""

        else:
            return PyCEGUI.WindowManager.getSingleton().getLayoutAsString(currentRootWidget)
*/
    return "";
}

bool LayoutCodeMode::propagateNativeCode(const QString& code)
{
    // We have to make the context the current context to ensure textures are fine
    /*
        mainwindow.MainWindow.instance.ceguiContainerWidget.makeGLContextCurrent()

        if code == "":
            self.tabbedEditor.visual.setRootWidget(None)

        else:
            try:
                newRoot = PyCEGUI.WindowManager.getSingleton().loadLayoutFromString(code)
                self.tabbedEditor.visual.setRootWidget(newRoot)

                return True

            except:
                return False
    */
    return true;
}

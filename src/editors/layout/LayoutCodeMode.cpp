#include "src/editors/layout/LayoutCodeMode.h"
#include "src/editors/layout/LayoutVisualMode.h"
#include "src/editors/layout/LayoutEditor.h"
#include "src/ui/layout/LayoutManipulator.h"
#include "src/cegui/CEGUIUtils.h"
#include <CEGUI/WindowManager.h>

LayoutCodeMode::LayoutCodeMode(LayoutEditor& editor)
    : ViewRestoringCodeEditMode(editor)
{
}

QString LayoutCodeMode::getNativeCode()
{
    const CEGUI::Window* rootWidget = static_cast<LayoutEditor&>(_editor).getVisualMode()->getRootWidget();
    if (!rootWidget) return "";
    return CEGUIUtils::stringToQString(CEGUI::WindowManager::getSingleton().getLayoutAsString(*rootWidget));
}

bool LayoutCodeMode::propagateNativeCode(const QString& code)
{
    LayoutVisualMode& visualMode = *static_cast<LayoutEditor&>(_editor).getVisualMode();

    if (code.isEmpty())
    {
        visualMode.setRootWidgetManipulator(nullptr);
    }
    else
    {
        try
        {
            CEGUI::Window* widget = CEGUI::WindowManager::getSingleton().loadLayoutFromString(CEGUIUtils::qStringToString(code));
            auto root = new LayoutManipulator(visualMode, nullptr, widget);
            root->updateFromWidget();
            root->createChildManipulators(true, false, false);
            visualMode.setRootWidgetManipulator(root);
        }
        catch (...)
        {
            return false;
        }
    }

    return true;
}

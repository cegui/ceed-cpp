#include "src/editors/layout/LayoutCodeMode.h"
#include "src/editors/layout/LayoutVisualMode.h"
#include "src/editors/layout/LayoutEditor.h"
#include "src/cegui/CEGUIProjectManager.h"
#include <CEGUI/WindowManager.h>

LayoutCodeMode::LayoutCodeMode(LayoutEditor& editor)
    : ViewRestoringCodeEditMode(editor)
{
}

QString LayoutCodeMode::getNativeCode()
{
    const CEGUI::Window* rootWidget = static_cast<LayoutEditor&>(_editor).getVisualMode()->getRootWidget();
    if (!rootWidget) return "";
    return ceguiStringToQString(CEGUI::WindowManager::getSingleton().getLayoutAsString(*rootWidget));
}

bool LayoutCodeMode::propagateNativeCode(const QString& code)
{
    // We have to make the context the current context to ensure textures are fine
    if (code.isEmpty())
    {
        static_cast<LayoutEditor&>(_editor).getVisualMode()->setRootWidget(nullptr);
    }
    else
    {
        try
        {
            CEGUI::Window* newRoot = CEGUI::WindowManager::getSingleton().loadLayoutFromString(qStringToCeguiString(code));
            static_cast<LayoutEditor&>(_editor).getVisualMode()->setRootWidget(newRoot);
        }
        catch (...)
        {
            return false;
        }
    }

    return true;
}

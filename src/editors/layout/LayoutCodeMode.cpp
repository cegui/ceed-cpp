#include "LayoutCodeMode.h"
#include "src/editors/layout/LayoutVisualMode.h"
#include "src/editors/layout/LayoutEditor.h"
#include "src/cegui/CEGUIUtils.h"
#include <CEGUI/WindowManager.h>

LayoutCodeMode::LayoutCodeMode(LayoutEditor& editor)
    : ViewRestoringCodeEditMode(editor)
{
}

QString LayoutCodeMode::getNativeCode()
{
    const CEGUI::Window* rootWidget = static_cast<LayoutEditor&>(_editor).getVisualMode()->getRootWidget();
    return rootWidget ? CEGUIUtils::stringToQString(CEGUI::WindowManager::getSingleton().getLayoutAsString(*rootWidget)) : "";
}

bool LayoutCodeMode::propagateNativeCode(const QString& code)
{
    return static_cast<LayoutEditor&>(_editor).loadVisualFromString(code);
}

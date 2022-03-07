#include "src/editors/layout/LayoutPreviewerMode.h"
#include "src/editors/layout/LayoutVisualMode.h"
#include "src/editors/layout/LayoutEditor.h"
#include "src/ui/CEGUIWidget.h"
#include "src/ui/CEGUIGraphicsScene.h"
#include "src/cegui/CEGUIManager.h" //!!!for OpenGL context! TODO: encapsulate?
#include "qboxlayout.h"
#include <CEGUI/Window.h>
#include <CEGUI/WindowManager.h>
#include <CEGUI/GUIContext.h>

LayoutPreviewerMode::LayoutPreviewerMode(LayoutEditor& editor, QWidget* parent)
    : QWidget(parent)
    , IEditMode(editor)
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    ceguiWidget = new CEGUIWidget(this);
    layout->addWidget(ceguiWidget);
    ceguiWidget->setScene(new CEGUIGraphicsScene());
    ceguiWidget->setViewFeatures(false, false, true, false);
    ceguiWidget->setInputEnabled(true);
}

void LayoutPreviewerMode::activate(MainWindow& mainWindow, bool editorActivated)
{
    IEditMode::activate(mainWindow, editorActivated);

    assert(!rootWidget);

    // Activate CEGUI OpenGL context for possible imagery cache FBOs creation
    CEGUIManager::Instance().makeOpenGLContextCurrent();

    // Lets clone so we don't affect the layout at all
    auto currentRootWidget = static_cast<LayoutEditor&>(_editor).getVisualMode()->getRootWidget();
    rootWidget = currentRootWidget ? currentRootWidget->clone() : nullptr;
    ceguiWidget->getScene()->getCEGUIContext()->setRootWindow(rootWidget);

    CEGUIManager::Instance().doneOpenGLContextCurrent();
}

bool LayoutPreviewerMode::deactivate(MainWindow& mainWindow, bool editorDeactivated)
{
    if (rootWidget)
    {
        CEGUI::WindowManager::getSingleton().destroyWindow(rootWidget);
        rootWidget = nullptr;
    }

    return IEditMode::deactivate(mainWindow, editorDeactivated);
}

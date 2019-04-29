#include "src/editors/layout/LayoutPreviewerMode.h"
#include "src/editors/layout/LayoutVisualMode.h"
#include "src/editors/layout/LayoutEditor.h"
#include "src/cegui/CEGUIProjectManager.h"
#include "src/ui/CEGUIGraphicsScene.h"
#include "src/ui/CEGUIWidget.h"
#include "qboxlayout.h"
#include <CEGUI/Window.h>
#include <CEGUI/WindowManager.h>

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
    ceguiWidget->setViewFeatures(false, false, true);
    ceguiWidget->setInputEnabled(true);
}

void LayoutPreviewerMode::activate()
{
    IEditMode::activate();

    assert(!rootWidget);

    // We have to make the context the current context to ensure textures are fine
    /*
    CEGUIProjectManager::Instance().makeOpenGLContextCurrent();
    */

    // Lets clone so we don't affect the layout at all
    auto currentRootWidget = static_cast<LayoutEditor&>(_editor).getVisualMode()->getRootWidget();
    rootWidget = currentRootWidget ? currentRootWidget->clone() : nullptr;
    ceguiWidget->getCEGUIContext()->setRootWindow(rootWidget);
}

bool LayoutPreviewerMode::deactivate()
{
    if (rootWidget)
    {
        CEGUI::WindowManager::getSingleton().destroyWindow(rootWidget);
        rootWidget = nullptr;
    }

    return IEditMode::deactivate();
}

void LayoutPreviewerMode::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    /*
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

#ifndef LAYOUTVISUALMODE_H
#define LAYOUTVISUALMODE_H

#include "src/editors/MultiModeEditor.h"
#include "qwidget.h"

// This is the layout visual editing mode

class LayoutEditor;
class LayoutScene;
class CreateWidgetDockWidget;
class WidgetHierarchyDockWidget;
class QMenu;

class LayoutVisualMode : public QWidget, public IEditMode
{
public:

    LayoutVisualMode(LayoutEditor& editor);

    void initialize();
    void rebuildEditorMenu(QMenu* editorMenu);

    bool cut();
    bool copy();
    bool paste();
    bool deleteSelected();

protected:

    void setupActions();
    void setupToolBar();

    virtual void showEvent(QShowEvent* event) override;
    virtual void hideEvent(QHideEvent* event) override;

    LayoutScene* scene = nullptr;
    CreateWidgetDockWidget* createWidgetDockWidget = nullptr;
    WidgetHierarchyDockWidget* hierarchyDockWidget = nullptr;
};

#endif // LAYOUTVISUALMODE_H

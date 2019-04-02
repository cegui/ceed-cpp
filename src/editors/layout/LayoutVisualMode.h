#ifndef LAYOUTVISUALMODE_H
#define LAYOUTVISUALMODE_H

#include "src/editors/MultiModeEditor.h"
#include "qwidget.h"

// This is the layout visual editing mode

class LayoutEditor;
class LayoutScene;
class CreateWidgetDockWidget;
class WidgetHierarchyDockWidget;
class QDockWidget;
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
    void zoomIn();
    void zoomOut();
    void zoomReset();

    //LayoutScene* getScene() const { return scene; }
    CreateWidgetDockWidget* getCreateWidgetDockWidget() const { return createWidgetDockWidget; }
    WidgetHierarchyDockWidget* getHierarchyDockWidget() const { return hierarchyDockWidget; }
    QDockWidget* getPropertiesDockWidget() const { return propertiesDockWidget; }

protected:

    void setupActions();
    void setupToolBar();

    virtual void showEvent(QShowEvent* event) override;
    virtual void hideEvent(QHideEvent* event) override;

    LayoutScene* scene = nullptr;
    CreateWidgetDockWidget* createWidgetDockWidget = nullptr;
    WidgetHierarchyDockWidget* hierarchyDockWidget = nullptr;
    QDockWidget* propertiesDockWidget = nullptr;
};

#endif // LAYOUTVISUALMODE_H

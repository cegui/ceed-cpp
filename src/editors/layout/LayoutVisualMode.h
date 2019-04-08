#ifndef LAYOUTVISUALMODE_H
#define LAYOUTVISUALMODE_H

#include "src/editors/MultiModeEditor.h"
#include "qwidget.h"

// This is the layout visual editing mode

class LayoutEditor;
class LayoutScene;
class LayoutManipulator;
class CreateWidgetDockWidget;
class WidgetHierarchyDockWidget;
class QDockWidget;
class QMenu;
class QToolBar;
class QAction;

class LayoutVisualMode : public QWidget, public IEditMode
{
public:

    LayoutVisualMode(LayoutEditor& editor);

    void initialize();
    void rebuildEditorMenu(QMenu* editorMenu);
    void setRootWidgetManipulator(LayoutManipulator* manipulator);

    bool cut();
    bool copy();
    bool paste();
    bool deleteSelected();
    void zoomIn();
    void zoomOut();
    void zoomReset();

    LayoutScene* getScene() const { return scene; }
    CreateWidgetDockWidget* getCreateWidgetDockWidget() const { return createWidgetDockWidget; }
    WidgetHierarchyDockWidget* getHierarchyDockWidget() const { return hierarchyDockWidget; }
    QDockWidget* getPropertiesDockWidget() const { return propertiesDockWidget; }
    QToolBar* getToolBar() const { return toolBar; }
    QAction* getAbsoluteModeAction() const { return absoluteModeAction; }

    bool isAbsoluteMode() const;

protected:

    void setupActions();
    void setupToolBar();

    void focusPropertyInspectorFilterBox();

    virtual void showEvent(QShowEvent* event) override;
    virtual void hideEvent(QHideEvent* event) override;

    LayoutScene* scene = nullptr;
    CreateWidgetDockWidget* createWidgetDockWidget = nullptr;
    WidgetHierarchyDockWidget* hierarchyDockWidget = nullptr;
    QDockWidget* propertiesDockWidget = nullptr;
    QToolBar* toolBar = nullptr;
    QMenu* _editorMenu = nullptr; // Not owned, just stored when we have control over its content
    QAction* absoluteModeAction = nullptr;
};

#endif // LAYOUTVISUALMODE_H

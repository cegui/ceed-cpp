#ifndef WIDGETHIERARCHYDOCKWIDGET_H
#define WIDGETHIERARCHYDOCKWIDGET_H

#include <QDockWidget>

// Displays and manages the widget hierarchy. Contains the WidgetHierarchyTreeWidget.

namespace Ui {
class WidgetHierarchyDockWidget;
}

class LayoutManipulator;
class LayoutVisualMode;
class QTreeView;

class WidgetHierarchyDockWidget : public QDockWidget
{
    Q_OBJECT

public:

    explicit WidgetHierarchyDockWidget(LayoutVisualMode& visualMode);
    virtual ~WidgetHierarchyDockWidget() override;

    void setupContextMenu();

    LayoutVisualMode& getVisualMode() const { return _visualMode; }
    void setRootWidgetManipulator(LayoutManipulator* root);
    void refresh();

    bool isIgnoringSelectionChanges() const { return _ignoreSelectionChanges; }
    void ignoreSelectionChanges(bool ignore) { _ignoreSelectionChanges = ignore; }
    void ignoreSelectionChangesInScene(bool ignore);

    QTreeView* getTreeView() const;

signals:

    void deleteRequested();

private:

    virtual void keyReleaseEvent(QKeyEvent* event) override;

    Ui::WidgetHierarchyDockWidget *ui;

    LayoutVisualMode& _visualMode;
    bool _ignoreSelectionChanges = false;
    LayoutManipulator* _rootWidgetManipulator = nullptr;
};

#endif // WIDGETHIERARCHYDOCKWIDGET_H

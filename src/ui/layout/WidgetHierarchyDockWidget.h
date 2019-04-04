#ifndef WIDGETHIERARCHYDOCKWIDGET_H
#define WIDGETHIERARCHYDOCKWIDGET_H

#include <QDockWidget>

// Displays and manages the widget hierarchy. Contains the WidgetHierarchyTreeWidget.

namespace Ui {
class WidgetHierarchyDockWidget;
}

class LayoutManipulator;

class WidgetHierarchyDockWidget : public QDockWidget
{
    Q_OBJECT

public:

    explicit WidgetHierarchyDockWidget(QWidget *parent = nullptr);
    ~WidgetHierarchyDockWidget();

    void setRootWidgetManipulator(LayoutManipulator* root);
    void refresh();

    bool isIgnoringSelectionChanges() const { return _ignoreSelectionChanges; }

private:

    virtual void keyReleaseEvent(QKeyEvent* event) override;

    Ui::WidgetHierarchyDockWidget *ui;

    bool _ignoreSelectionChanges = false;
    LayoutManipulator* _rootWidgetManipulator = nullptr;
};

#endif // WIDGETHIERARCHYDOCKWIDGET_H

#ifndef WIDGETHIERARCHYDOCKWIDGET_H
#define WIDGETHIERARCHYDOCKWIDGET_H

#include <QDockWidget>

// Displays and manages the widget hierarchy. Contains the WidgetHierarchyTreeWidget.

namespace Ui {
class WidgetHierarchyDockWidget;
}

class WidgetHierarchyDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit WidgetHierarchyDockWidget(QWidget *parent = nullptr);
    ~WidgetHierarchyDockWidget();

private:
    Ui::WidgetHierarchyDockWidget *ui;
};

#endif // WIDGETHIERARCHYDOCKWIDGET_H

#include "src/ui/layout/WidgetHierarchyDockWidget.h"
#include "ui_WidgetHierarchyDockWidget.h"
#include "src/ui/layout/WidgetHierarchyTreeModel.h"
#include "qevent.h"

WidgetHierarchyDockWidget::WidgetHierarchyDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::WidgetHierarchyDockWidget)
{
    ui->setupUi(this);

    auto model = new WidgetHierarchyTreeModel(this);
    ui->treeView->setModel(model);
/*
        self.visual = visual
*/
}

WidgetHierarchyDockWidget::~WidgetHierarchyDockWidget()
{
    delete ui;
}

// Sets the widget manipulator that is at the root of our observed hierarchy.
// Uses getTreeItemForManipulator to recursively populate the tree.
void WidgetHierarchyDockWidget::setRootWidgetManipulator(LayoutManipulator* root)
{
    _rootWidgetManipulator = root;
/*
        self.model.setRootManipulator(root)
*/
    ui->treeView->expandToDepth(0);
}

// Refreshes the entire hierarchy completely from scratch
void WidgetHierarchyDockWidget::refresh()
{
    // This will resynchronise the entire model
    /*
            self.model.setRootManipulator(_rootWidgetManipulator)
    */
}

void WidgetHierarchyDockWidget::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete)
    {
        /*
        handled = self.visual.scene.deleteSelectedWidgets()

        if handled:
            return True
        */
    }

    return QDockWidget::keyReleaseEvent(event);
}
